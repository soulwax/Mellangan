#include "level/world/Internal.hpp"

namespace world {

namespace {

constexpr uint8_t rimOffset(int mask) {
  constexpr uint8_t edge[4] = {1, 2, 0, 1};
  return uint8_t(edge[mask & 3] * 12 + edge[mask >> 2 & 3]);
}
constexpr uint8_t RIM[16] = {
    rimOffset(0),  rimOffset(1),  rimOffset(2),  rimOffset(3),
    rimOffset(4),  rimOffset(5),  rimOffset(6),  rimOffset(7),
    rimOffset(8),  rimOffset(9),  rimOffset(10), rimOffset(11),
    rimOffset(12), rimOffset(13), rimOffset(14), rimOffset(15)};

bool winterAt(int x, int y) { return BIOMES[biomeAt(x, y)]->winter(); }

template <class F> int sides(int x, int y, F at) {
  return int(at(x, y - 1)) | at(x, y + 1) << 1 | at(x - 1, y) << 2 |
         at(x + 1, y) << 3;
}

} // namespace

int rim(int mask) { return RIM[mask & 15]; }

Sprite groundTile(int x, int y) {
  if (active != Layer::Surface)
    return under::groundTile(x, y);
  Structure st;
  Sprite out;
  const Biome *b = BIOMES[biomeAt(x, y)];
  if (structureAt(x, y, st) && structureGround(&st, x - st.x, y - st.y, &out))
    return b->frost(out);
  if (isWater(x, y))
    return b->lakeRim(RIM[sides(x, y, isWater)]);
  if (isDirt(x, y))
    return b->dirtRim(RIM[sides(x, y, isDirt)]);
  if (b->ground(x, y, &out))
    return out;
  if (b->winter())
    if (int snow = sides(x, y, winterAt); snow != 15)
      return atlas::snowRim(RIM[snow]);
  const Biome *d = blendedBiome(x, y);
  float n = groundShade(x, y);
  int r = hash2(x, y) % 100, flowers = int(d->flowers * (0.15f + 2.2f * n * n)),
      tufts = int(d->tufts * (0.5f + n));
  return d->grass(r < flowers ? 2 : r < flowers + tufts ? 1 : 0);
}

float groundShade(int x, int y) {
  return memo(x, y, slot::shade, &TileMemo::shade, [&] {
    return noise_unit(noise_fbm(x * 0.13f + 0.5f, y * 0.13f + 0.5f, 2, 606) * 1.6f);
  });
}

Color groundTint(int x, int y) {
  if (active != Layer::Surface)
    return under::groundTint(x, y);
  GroundBlend blend = groundBlend(x, y);
  Color out{0, 0, 0, 1};
  for (int i = 0; i < blend.count; i++) {
    const Color &t = BIOMES[blend.biome[i]]->tint;
    out.r += t.r * blend.weight[i];
    out.g += t.g * blend.weight[i];
    out.b += t.b * blend.weight[i];
  }
  return out;
}

} // namespace world
