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
    Slot *c = &cache[hash2(rx, ry) & 255];
    if (!c->used || c->rx != rx || c->ry != ry) c->rx = rx, c->ry = ry, c->used = true, c->has = rollStructure(rx, ry, &c->s);
    if (c->has) s = c->s;
    return c->has;
}

bool structureAt(int x, int y, Structure &out, int margin) {
    return structureIn(regionOf(x), regionOf(y), out) && out.contains(x, y, margin);
}

bool nearestStructure(Vec2 p, int maxRegions, Structure &out) {
    int tx = int(std::floor(p.x / 16)), ty = int(std::floor(p.y / 16)), rx = regionOf(tx), ry = regionOf(ty);
    float best = INFINITY;
    Structure s;
    for (int y = ry - maxRegions; y <= ry + maxRegions; y++)
        for (int x = rx - maxRegions; x <= rx + maxRegions; x++) {
            if (!structureIn(x, y, s) || s.contains(tx, ty)) continue;
            float d = lengthSq(s.center() - p);
            if (d < best) best = d, out = s;
        }
    return best < INFINITY;
}

bool isDirt(int x, int y) {
    Structure s;
    if (structureAt(x, y, s, 1)) return structureDirt(&s, x - s.x, y - s.y);
    return dirtNoise(x, y, &BIOMES[biomeAt(x, y)]);
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
    bool n = isDirt(x, y - 1), s = isDirt(x, y + 1), w = isDirt(x - 1, y), e = isDirt(x + 1, y);
    int row = n == s ? 1 : n ? 2 : 0, col = w == e ? 1 : w ? 2 : 0;
    return atlas::town(tt::DirtTopLeft + row * 12 + col);
}

float groundShade(int x, int y) { return noise_unit(noise_fbm(x * 0.13f + 0.5f, y * 0.13f + 0.5f, 2, 606) * 1.6f); }

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
    float clump = clumpAt(x, y);
    for (int i = 0; i < MAX_SPAWNS && b->spawns[i].feature; i++) {
        const FeatureSpawn *s = &b->spawns[i];
        int chance = s->base + int(s->dense * clump);
        if (r < chance) return s->feature->place(s->feature, h >> 16, &p);
        r -= chance;
    }
    return false;
}

} // namespace world
