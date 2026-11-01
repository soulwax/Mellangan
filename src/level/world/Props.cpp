#include "level/world/Internal.hpp"

namespace world {

namespace {

namespace fe = feature;

const Feature *const ROCKS[5] = {&fe::boulder, &fe::mossyBoulder, &fe::snowyBoulder,
                                 &fe::sandstone, &fe::obsidian};
const uint8_t ROCK[BIOME_COUNT] = {1, 1, 0, 0, 0, 2, 1, 1, 3, 1, 3, 4, 0,
                                   1, 3, 2, 3, 1, 0, 0, 4, 3, 0, 3, 1, 0};
const int HIGH_ROCKS[4] = {0, 0, 14, 45}, HIGH_COVER[4] = {0, 0, 20, 60},
          THIN[4] = {0, 0, 56, 100};

bool place(const Feature *f, uint32_t h, Prop &p) {
  return f->place(f, h >> 16, &p);
}

bool wildProp(int x, int y, uint32_t h, Prop &p) {
  const Biome *self = BIOMES[biomeAt(x, y)];
  if (int own = self->place(x, y, h, p); own >= 0)
    return own;
  uint32_t g = hash2(x * 31 + 5, y * 17 + 9);
  const Biome *other = blendPick(x, y, g);
  if (other != self && (h >> 4) % 1000 < 110) {
    const Feature *fringe = (h >> 21) & 1 ? self->fringe : other->fringe;
    if (fringe)
      return place(fringe, h, p);
  }
  const Biome *b = blendedBiome(x, y);
  int r = (h >> 4) % 1000;
  float clump = memo(x, y, slot::clump, &TileMemo::clump,
                     [&] { return clumpAt(x, y); });
  for (int i = 0; i < MAX_SPAWNS && b->spawns[i].feature; i++) {
    const FeatureSpawn *s = &b->spawns[i];
    int chance = s->base + int(s->dense * clump);
    if (r < chance)
      return place(s->feature, h, p);
    r -= chance;
  }
  return false;
}

bool clearing(int x, int y, uint32_t h) {
  if (std::abs(x) >= 9 || std::abs(y) >= 9)
    return false;
  float d = std::sqrt(float(x) * x + float(y) * y),
        w = d + 1.5f * noise_fbm(x * 0.2f - 5.3f, y * 0.2f + 8.1f, 2, 1213);
  return d < 2.5f || w < 3.5f ||
         (w < 6.5f && float(h >> 22 & 1023) < (6.5f - w) / 3 * 1024);
}

} // namespace

bool propAt(int x, int y, Prop &p) {
  p = Prop{};
  p.x = x * 16 + 8.0;
  p.y = y * 16 + 16.0;

  Structure st;
  if (structureAt(x, y, st, 1))
    return structureProp(&st, x - st.x, y - st.y, &p);
  if (isDirt(x, y) || isWater(x, y) || isRamp(x, y) || isRamp(x, y - 1))
    return false;

  uint32_t h = hash2(x * 7 + 3, y * 13 + 1);
  if (clearing(x, y, h))
    return false;
  int level = heightAt(x, y), id = biomeAt(x, y);
  uint32_t q = hash2(x * 59 + 1, y * 43 - 7);
  const Feature *rock = ROCKS[ROCK[id]];
  if (heightAt(x, y - 1) > level || heightAt(x - 1, y) > level) {
    if (q % 100 < 6)
      return place(rock, h, p);
    if (q % 100 < 24)
      return place(&fe::pebbles, h, p);
  }
  if (level >= 2) {
    int roll = int(q % 1000);
    if (roll < HIGH_ROCKS[level])
      return place(rock, h, p);
    if (roll < HIGH_ROCKS[level] + HIGH_COVER[level])
      return place(ROCK[id] < 2 && q >> 20 & 1 ? &fe::heather : &fe::pebbles, h, p);
  }
  if (!wildProp(x, y, h, p))
    return false;
  return !(p.solid > 0 && int(q >> 12 & 127) < THIN[level]);
}

} // namespace world
