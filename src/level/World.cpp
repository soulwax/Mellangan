#include "level/World.hpp"

#include "level/Noise.h"

namespace world {

namespace {

namespace tt = atlas::tt;

StructureKind pickKind(const BiomeDef *b, uint32_t roll) {
    int total = 0;
    for (int k = 0; k < int(StructureKind::Count); k++) total += b->structureWeights[k];
    int r = int(roll % uint32_t(total));
    for (int k = 0; k < int(StructureKind::Count); k++)
        if ((r -= b->structureWeights[k]) < 0) return StructureKind(k);
    return StructureKind::Cottage;
}

bool dirtNoise(int x, int y, const BiomeDef *b) { return noise_fbm(x * 0.11f - 200.5f, y * 0.11f + 77.5f, 2, 505) > b->dirt; }

// Per-tile memo of everything noise-derived. bits holds the biome in its low 3 bits plus has/value flags, so one
// direct-mapped slot answers biome, dirt, shade and clump. Indexed by the low coordinate bits: any 128x64-tile
// window maps without collisions, which covers the view, its margins and the tint's +-3 taps.
enum : uint32_t { BIOME_BITS = 7, HAS_BIOME = 1 << 3, HAS_DIRT = 1 << 4, DIRT = 1 << 5, HAS_SHADE = 1 << 6, HAS_CLUMP = 1 << 7 };
struct TileMemo {
    int32_t x, y;
    uint32_t bits;
    float shade, clump;
};
TileMemo memo[128 * 64];

TileMemo *memoAt(int x, int y) {
    TileMemo *m = &memo[(x & 127) | (y & 63) << 7];
    if (m->x != x || m->y != y) m->x = x, m->y = y, m->bits = 0;
    return m;
}

// Dirt rim tile offset from a 4-bit neighbour mask (n, s, w, e): a lone side picks that edge, both or neither the middle.
constexpr uint8_t rimOffset(int mask) {
    constexpr uint8_t edge[4] = {1, 2, 0, 1};
    return uint8_t(edge[mask & 3] * 12 + edge[mask >> 2 & 3]);
}
constexpr uint8_t RIM[16] = {rimOffset(0), rimOffset(1), rimOffset(2),  rimOffset(3),  rimOffset(4),  rimOffset(5),
                             rimOffset(6), rimOffset(7), rimOffset(8),  rimOffset(9),  rimOffset(10), rimOffset(11),
                             rimOffset(12), rimOffset(13), rimOffset(14), rimOffset(15)};

bool rollStructure(int rx, int ry, Structure *out) {
    uint32_t h = hash2(rx * 31 + 7, ry * 17 - 3);
    int cx = rx * REGION + REGION / 2, cy = ry * REGION + REGION / 2;
    const BiomeDef *b = &BIOMES[biomeAt(cx, cy)];
    if (int(h % 100) >= b->structureChance) return false;

    StructureKind kind = pickKind(b, h >> 8);
    int w, ht;
    STRUCTURES[int(kind)].size(h, &w, &ht);
    int spanX = REGION - 4 - w, spanY = REGION - 4 - ht;
    int x = rx * REGION + 2 + int((h >> 18) % uint32_t(spanX + 1)), y = ry * REGION + 2 + int((h >> 24) % uint32_t(spanY + 1));
    if (x - 1 < 6 && x + w + 1 > -6 && y - 1 < 6 && y + ht + 1 > -6) return false; // keep the spawn clear

    *out = {kind, x, y, w, ht, rx, ry, hash2(int(h), rx ^ (ry << 8))};
    return true;
}

} // namespace

bool structureIn(int rx, int ry, Structure &s) {
    struct Slot {
        int rx, ry;
        bool used, has;
        Structure s;
    };
    static Slot cache[256];
    Slot *c = &cache[(rx & 15) | (ry & 15) << 4];
    if (!c->used || c->rx != rx || c->ry != ry) c->rx = rx, c->ry = ry, c->used = true, c->has = rollStructure(rx, ry, &c->s);
    if (c->has) s = c->s;
    return c->has;
}

bool structureAt(int x, int y, Structure &out, int margin) {
    return structureIn(regionOf(x), regionOf(y), out) && out.contains(x, y, margin);
}

bool nearestStructure(Vec2 p, int maxRegions, Structure &out, StructureFilter keep, const void *ctx) {
    int tx = int(std::floor(p.x / 16)), ty = int(std::floor(p.y / 16)), rx = regionOf(tx), ry = regionOf(ty);
    float best = INFINITY;
    Structure s;
    for (int y = ry - maxRegions; y <= ry + maxRegions; y++)
        for (int x = rx - maxRegions; x <= rx + maxRegions; x++) {
            if (!structureIn(x, y, s) || s.contains(tx, ty) || (keep && !keep(&s, ctx))) continue;
            float d = lengthSq(s.center() - p);
            if (d < best) best = d, out = s;
        }
    return best < INFINITY;
}

BiomeId biomeAt(int x, int y) {
    TileMemo *m = memoAt(x, y);
    if (!(m->bits & HAS_BIOME)) m->bits |= HAS_BIOME | uint32_t(biomeOf(climateAt(x, y)));
    return BiomeId(m->bits & BIOME_BITS);
}

bool isDirt(int x, int y) {
    TileMemo *m = memoAt(x, y);
    if (!(m->bits & HAS_DIRT)) {
        Structure s;
        bool dirt = structureAt(x, y, s, 1) ? structureDirt(&s, x - s.x, y - s.y) : dirtNoise(x, y, &BIOMES[biomeAt(x, y)]);
        m->bits |= HAS_DIRT | (dirt ? DIRT : 0);
    }
    return m->bits & DIRT;
}

Sprite groundTile(int x, int y) {
    Structure st;
    Sprite out;
    if (structureAt(x, y, st) && structureGround(&st, x - st.x, y - st.y, &out)) return out;
    if (!isDirt(x, y)) {
        const BiomeDef *b = &BIOMES[biomeAt(x, y)];
        int r = hash2(x, y) % 100;
        return atlas::town(r < b->flowers ? tt::GrassFlowers : r < b->flowers + b->tufts ? tt::GrassTufts : tt::Grass);
    }
    int mask = int(isDirt(x, y - 1)) | isDirt(x, y + 1) << 1 | isDirt(x - 1, y) << 2 | isDirt(x + 1, y) << 3;
    return atlas::town(tt::DirtTopLeft + RIM[mask]);
}

float groundShade(int x, int y) {
    TileMemo *m = memoAt(x, y);
    if (!(m->bits & HAS_SHADE)) m->bits |= HAS_SHADE, m->shade = noise_unit(noise_fbm(x * 0.13f + 0.5f, y * 0.13f + 0.5f, 2, 606) * 1.6f);
    return m->shade;
}

Color groundTint(int x, int y) {
    // Average a plus of samples so tints fade across a few tiles instead of stepping at the border.
    static const int OFFSETS[5][2] = {{0, 0}, {-3, 0}, {3, 0}, {0, -3}, {0, 3}};
    Color c{0, 0, 0, 1};
    for (int i = 0; i < 5; i++) {
        const Color *t = &BIOMES[biomeAt(x + OFFSETS[i][0], y + OFFSETS[i][1])].tint;
        c.r += t->r * 0.2f, c.g += t->g * 0.2f, c.b += t->b * 0.2f;
    }
    return c;
}

bool propAt(int x, int y, Prop &p) {
    p = Prop{};
    p.pos = {x * 16 + 8.0f, y * 16 + 16.0f};

    Structure st;
    if (structureAt(x, y, st, 1)) return structureProp(&st, x - st.x, y - st.y, &p);
    if ((std::abs(x) < 4 && std::abs(y) < 4) || isDirt(x, y)) return false;

    const BiomeDef *b = &BIOMES[biomeAt(x, y)];
    uint32_t h = hash2(x * 7 + 3, y * 13 + 1);
    int r = (h >> 4) % 1000;
    TileMemo *m = memoAt(x, y);
    if (!(m->bits & HAS_CLUMP)) m->bits |= HAS_CLUMP, m->clump = clumpAt(x, y);
    float clump = m->clump;
    for (int i = 0; i < MAX_SPAWNS && b->spawns[i].feature; i++) {
        const FeatureSpawn *s = &b->spawns[i];
        int chance = s->base + int(s->dense * clump);
        if (r < chance) return s->feature->place(s->feature, h >> 16, &p);
        r -= chance;
    }
    return false;
}

} // namespace world
