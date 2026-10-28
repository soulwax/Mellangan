#include "level/World.hpp"

#include "level/Noise.h"

namespace world {

namespace {

namespace tt = atlas::tt;

StructureKind pickKind(const Biome *b, uint32_t roll) {
  int total = 0;
  for (int k = 0; k < int(StructureKind::Count); k++)
    total += b->structureWeights[k];
  int r = int(roll % uint32_t(total));
  for (int k = 0; k < int(StructureKind::Count); k++)
    if ((r -= b->structureWeights[k]) < 0)
      return StructureKind(k);
  return StructureKind::Cottage;
}

bool dirtNoise(int x, int y, const Biome *b) {
  return noise_fbm(x * 0.11f - 200.5f, y * 0.11f + 77.5f, 2, 505) > b->dirt;
}

// Per-tile memo of everything noise-derived. bits holds the biome in its low 3
// bits plus has/value flags, so one direct-mapped slot answers biome, dirt,
// shade and clump. Indexed by the low coordinate bits: any 128x64-tile window
// maps without collisions, which covers the view, its margins and the tint's
// +-3 taps.
enum : uint32_t {
  BIOME_BITS = 31,
  HAS_BIOME = 1 << 5,
  HAS_DIRT = 1 << 6,
  DIRT = 1 << 7,
  HAS_SHADE = 1 << 8,
  HAS_CLUMP = 1 << 9,
  HAS_RAW = 1 << 10,
  HAS_RAW_DIRT = 1 << 11,
  RAW_DIRT = 1 << 12,
  RAW_SHIFT = 13
};
struct TileMemo {
  int32_t x, y;
  uint32_t bits, epoch;
  float shade, clump;
};
TileMemo memo[128 * 64];

TileMemo *memoAt(int x, int y) {
  TileMemo *m = &memo[(x & 127) | (y & 63) << 7];
  if (m->epoch != noise_epoch || m->x != x || m->y != y)
    m->x = x, m->y = y, m->bits = 0, m->epoch = noise_epoch;
  return m;
}

BiomeId rawBiome(int x, int y) {
  TileMemo *m = memoAt(x, y);
  if (!(m->bits & HAS_RAW))
    m->bits |= HAS_RAW | uint32_t(biomeOf(climateAt(x, y))) << RAW_SHIFT;
  return BiomeId(m->bits >> RAW_SHIFT & BIOME_BITS);
}

bool rawDirt(int x, int y) {
  TileMemo *m = memoAt(x, y);
  if (!(m->bits & HAS_RAW_DIRT))
    m->bits |= HAS_RAW_DIRT |
               (dirtNoise(x, y, BIOMES[biomeAt(x, y)]) ? uint32_t(RAW_DIRT) : 0u);
  return m->bits & RAW_DIRT;
}

bool rawDirtAt(int x, int y, const void *) { return rawDirt(x, y); }

bool patchDirt(int x, int y) { return smoothPatch(rawDirtAt, nullptr, x, y); }

// Dirt rim tile offset from a 4-bit neighbour mask (n, s, w, e): a lone side
// picks that edge, both or neither the middle.
constexpr uint8_t rimOffset(int mask) {
  constexpr uint8_t edge[4] = {1, 2, 0, 1};
  return uint8_t(edge[mask & 3] * 12 + edge[mask >> 2 & 3]);
}
constexpr uint8_t RIM[16] = {
    rimOffset(0),  rimOffset(1),  rimOffset(2),  rimOffset(3),
    rimOffset(4),  rimOffset(5),  rimOffset(6),  rimOffset(7),
    rimOffset(8),  rimOffset(9),  rimOffset(10), rimOffset(11),
    rimOffset(12), rimOffset(13), rimOffset(14), rimOffset(15)};

bool rollStructure(int rx, int ry, Structure *out) {
  uint32_t h = hash2(rx * 31 + 7, ry * 17 - 3);
  int cx = rx * REGION + REGION / 2, cy = ry * REGION + REGION / 2;
  const Biome *b = BIOMES[biomeAt(cx, cy)];
  if (int(h % 100) >= b->structureChance)
    return false;

  StructureKind kind = pickKind(b, h >> 8);
  int w, ht;
  STRUCTURES[int(kind)].size(h, &w, &ht);
  int spanX = REGION - 4 - w, spanY = REGION - 4 - ht;
  int x = rx * REGION + 2 + int((h >> 18) % uint32_t(spanX + 1)),
      y = ry * REGION + 2 + int((h >> 24) % uint32_t(spanY + 1));
  if (x - 1 < 6 && x + w + 1 > -6 && y - 1 < 6 && y + ht + 1 > -6)
    return false; // keep the spawn clear

  *out = {kind, x, y, w, ht, rx, ry, hash2(int(h), rx ^ (ry << 8))};
  return true;
}

} // namespace

bool structureIn(int rx, int ry, Structure &s) {
  struct Slot {
    int rx, ry;
    uint32_t epoch;
    bool has;
    Structure s;
  };
  static Slot cache[256];
  Slot *c = &cache[(rx & 15) | (ry & 15) << 4];
  if (c->epoch != noise_epoch || c->rx != rx || c->ry != ry)
    c->rx = rx, c->ry = ry, c->epoch = noise_epoch,
    c->has = rollStructure(rx, ry, &c->s);
  if (c->has)
    s = c->s;
  return c->has;
}

bool structureAt(int x, int y, Structure &out, int margin) {
  return structureIn(regionOf(x), regionOf(y), out) &&
         out.contains(x, y, margin);
}

bool nearestStructure(double x, double y, int maxRegions, Structure &out,
                      StructureFilter keep, const void *ctx) {
  int tx = int(std::floor(x / 16)), ty = int(std::floor(y / 16)),
      rx = regionOf(tx), ry = regionOf(ty);
  double best = INFINITY;
  Structure s;
  for (int yr = ry - maxRegions; yr <= ry + maxRegions; yr++)
    for (int xr = rx - maxRegions; xr <= rx + maxRegions; xr++) {
      if (!structureIn(xr, yr, s) || s.contains(tx, ty) ||
          (keep && !keep(&s, ctx)))
        continue;
      double xd = s.centerX() - x, yd = s.centerY() - y, d = xd * xd + yd * yd;
      if (d < best)
        best = d, out = s;
    }
  return best < INFINITY;
}

BiomeId biomeAt(int x, int y) {
  TileMemo *m = memoAt(x, y);
  if (!(m->bits & HAS_BIOME)) {
    uint8_t votes[BIOME_COUNT]{};
    BiomeId own = rawBiome(x, y), best = own;
    for (int dy = -1; dy <= 1; dy++)
      for (int dx = -1; dx <= 1; dx++) {
        BiomeId b = rawBiome(x + dx, y + dy);
        if (++votes[b] > votes[best] || (b == own && votes[b] == votes[best]))
          best = b;
      }
    m = memoAt(x, y);
    m->bits |= HAS_BIOME | uint32_t(best);
  }
  return BiomeId(m->bits & BIOME_BITS);
}

bool solidCore(PatchTest test, const void *ctx, int x, int y) {
  if (!test(x, y, ctx))
    return false;
  for (int dy = -1; dy <= 0; dy++)
    for (int dx = -1; dx <= 0; dx++)
      if (test(x + dx, y + dy, ctx) && test(x + dx + 1, y + dy, ctx) &&
          test(x + dx, y + dy + 1, ctx) && test(x + dx + 1, y + dy + 1, ctx))
        return true;
  return false;
}

bool smoothPatch(PatchTest test, const void *ctx, int x, int y) {
  if (solidCore(test, ctx, x, y))
    return true;
  int n = solidCore(test, ctx, x, y - 1) + solidCore(test, ctx, x, y + 1) +
          solidCore(test, ctx, x - 1, y) + solidCore(test, ctx, x + 1, y);
  return n >= 3;
}

bool isDirt(int x, int y) {
  TileMemo *m = memoAt(x, y);
  if (!(m->bits & HAS_DIRT)) {
    Structure s;
    bool dirt = structureAt(x, y, s, 1) ? structureDirt(&s, x - s.x, y - s.y)
                                        : patchDirt(x, y);
    m->bits |= HAS_DIRT | (dirt ? uint32_t(DIRT) : 0u);
  }
  return m->bits & DIRT;
}

int rim(int mask) { return RIM[mask & 15]; }

bool winterAt(int x, int y) { return BIOMES[biomeAt(x, y)]->winter(); }

const Biome *blendedBiome(int x, int y) {
  const Biome *here = BIOMES[biomeAt(x, y)];
  uint32_t h = hash2(x * 31 + 5, y * 17 + 9);
  const Biome *there =
      BIOMES[biomeAt(x + int(h % 7) - 3, y + int((h >> 3) % 7) - 3)];
  return there->winter() == here->winter() ? there : here;
}

Sprite groundTile(int x, int y) {
  Structure st;
  Sprite out;
  const Biome *b = BIOMES[biomeAt(x, y)];
  if (structureAt(x, y, st) && structureGround(&st, x - st.x, y - st.y, &out))
    return b->frost(out);
  if (!isDirt(x, y)) {
    if (b->ground(x, y, &out))
      return out;
    if (b->winter()) {
      int snow = int(winterAt(x, y - 1)) | winterAt(x, y + 1) << 1 |
                 winterAt(x - 1, y) << 2 | winterAt(x + 1, y) << 3;
      if (snow != 15)
        return atlas::snowRim(RIM[snow]);
    }
    const Biome *d = blendedBiome(x, y);
    int r = hash2(x, y) % 100;
    return d->grass(r < d->flowers ? 2 : r < d->flowers + d->tufts ? 1 : 0);
  }
  int mask = int(isDirt(x, y - 1)) | isDirt(x, y + 1) << 1 |
             isDirt(x - 1, y) << 2 | isDirt(x + 1, y) << 3;
  return b->dirtRim(RIM[mask]);
}

float groundShade(int x, int y) {
  TileMemo *m = memoAt(x, y);
  if (!(m->bits & HAS_SHADE))
    m->bits |= HAS_SHADE,
        m->shade = noise_unit(
            noise_fbm(x * 0.13f + 0.5f, y * 0.13f + 0.5f, 2, 606) * 1.6f);
  return m->shade;
}

Color groundTint(int x, int y) {
  int gx = x >> 2, gy = y >> 2;
  float fx = ((x & 3) + 0.5f) / 4, fy = ((y & 3) + 0.5f) / 4;
  fx = fx * fx * (3 - 2 * fx), fy = fy * fy * (3 - 2 * fy);
  const Color &a = BIOMES[biomeAt(gx * 4, gy * 4)]->tint,
              &b = BIOMES[biomeAt(gx * 4 + 4, gy * 4)]->tint,
              &c = BIOMES[biomeAt(gx * 4, gy * 4 + 4)]->tint,
              &d = BIOMES[biomeAt(gx * 4 + 4, gy * 4 + 4)]->tint;
  auto mix = [&](float Color::*k) {
    float top = lerpf(a.*k, b.*k, fx), bottom = lerpf(c.*k, d.*k, fx);
    return lerpf(top, bottom, fy);
  };
  return {mix(&Color::r), mix(&Color::g), mix(&Color::b), 1};
}

bool propAt(int x, int y, Prop &p) {
  p = Prop{};
  p.x = x * 16 + 8.0;
  p.y = y * 16 + 16.0;

  Structure st;
  if (structureAt(x, y, st, 1))
    return structureProp(&st, x - st.x, y - st.y, &p);
  if ((std::abs(x) < 4 && std::abs(y) < 4) || isDirt(x, y))
    return false;

  uint32_t h = hash2(x * 7 + 3, y * 13 + 1);
  const Biome *self = BIOMES[biomeAt(x, y)];
  if (int own = self->place(x, y, h, p); own >= 0)
    return own;
  uint32_t g = hash2(x * 31 + 5, y * 17 + 9);
  const Biome *other =
      BIOMES[biomeAt(x + int(g % 7) - 3, y + int((g >> 3) % 7) - 3)];
  if (other != self && (h >> 4) % 1000 < 110) {
    const Feature *fringe = (h >> 21) & 1 ? self->fringe : other->fringe;
    if (fringe)
      return fringe->place(fringe, h >> 16, &p);
  }
  const Biome *b = blendedBiome(x, y);
  int r = (h >> 4) % 1000;
  TileMemo *m = memoAt(x, y);
  if (!(m->bits & HAS_CLUMP))
    m->bits |= HAS_CLUMP, m->clump = clumpAt(x, y);
  float clump = m->clump;
  for (int i = 0; i < MAX_SPAWNS && b->spawns[i].feature; i++) {
    const FeatureSpawn *s = &b->spawns[i];
    int chance = s->base + int(s->dense * clump);
    if (r < chance)
      return s->feature->place(s->feature, h >> 16, &p);
    r -= chance;
  }
  return false;
}

} // namespace world
