#include "level/world/Internal.hpp"

namespace world {

const float LAKES[BIOME_COUNT] = {
    0,     0.04f, 0.02f, -0.06f, 0,     0,     0.02f, 0.16f, -1,
    0.08f, -0.12f, -1,   0,      0.06f, -1,    0.04f, -1,    0.2f,
    0.06f, 0.14f, 0.08f, 0.02f,  0,     -1,    0.06f, 0};

namespace {

bool dirtNoise(int x, int y, const Biome *b) {
  return noise_fbm(x * 0.11f - 200.5f, y * 0.11f + 77.5f, 2, 505) > b->dirt;
}

int levelNoise(int x, int y) {
  float wx = x + 9 * noise_fbm(x * 0.04f + 17.3f, y * 0.04f - 2.9f, 2, 1303),
        wy = y + 9 * noise_fbm(x * 0.04f - 31.1f, y * 0.04f + 8.7f, 2, 1305);
  float n = noise_fbm(wx * 0.0075f - 91.3f, wy * 0.0075f + 57.1f, 3, 1301);
  float d = std::sqrt(float(x) * x + float(y) * y);
  float lift = clampf((d - 18) / 42, 0, 1);
  n *= lift * lift * (3 - 2 * lift);
  static const float STEPS[3] = {0.14f, 0.34f, 0.48f};
  int level = 0;
  while (level < 3 &&
         n > STEPS[level] + 0.06f * noise_fbm(x * 0.09f + level * 13.7f,
                                               y * 0.09f - level * 5.3f, 2,
                                               1307 + level))
    level++;
  return level;
}

float spawnDry(int x, int y) {
  float d = std::sqrt(float(x) * x + float(y) * y) +
            6 * noise_fbm(x * 0.09f + 3.1f, y * 0.09f - 7.7f, 2, 1207);
  float k = clampf(1 - (d - 7) / 18, 0, 1);
  return 1.6f * k * k * (3 - 2 * k);
}

float structureDry(int x, int y) {
  constexpr int REACH = 6;
  int rx = regionOf(x), ry = regionOf(y), lx = x - rx * REGION, ly = y - ry * REGION;
  float best = 0;
  Structure s;
  for (int yr = ry - (ly < REACH); yr <= ry + (ly >= REGION - REACH); yr++)
    for (int xr = rx - (lx < REACH); xr <= rx + (lx >= REGION - REACH); xr++) {
      if (!structureIn(xr, yr, s))
        continue;
      float dx = float(std::max({s.x - x, 0, x - (s.x + s.w - 1)})),
            dy = float(std::max({s.y - y, 0, y - (s.y + s.h - 1)}));
      if (dx > REACH || dy > REACH)
        continue;
      float d = std::sqrt(dx * dx + dy * dy) +
                2.5f * noise_fbm(x * 0.25f + 11.3f, y * 0.25f - 4.9f, 2, 1211);
      float k = clampf(1 - (d - 1) / 3.5f, 0, 1);
      best = std::max(best, 1.6f * k * k * (3 - 2 * k));
    }
  return best;
}

bool tarn(int x, int y, int level) {
  if (LAKES[biomeAt(x, y)] < -0.5f ||
      noise_fbm(x * 0.07f + 23.1f, y * 0.07f - 9.4f, 2, 1223) > -0.36f)
    return false;
  for (int dy = -2; dy <= 2; dy++)
    for (int dx = -2; dx <= 2; dx++)
      if (heightAt(x + dx, y + dy) != level)
        return false;
  return true;
}

bool rawDirtAt(int x, int y, const void *) {
  return memo(x, y, slot::rawDirt,
              [&] { return dirtNoise(x, y, BIOMES[biomeAt(x, y)]); });
}

int rawLevel(int x, int y) {
  return int(memo(x, y, slot::rawLevel, [&] { return levelNoise(x, y); }));
}

bool rawAbove(int x, int y, const void *k) {
  return rawLevel(x, y) >= *static_cast<const int *>(k);
}

bool rawBelow(int x, int y, const void *k) { return !rawAbove(x, y, k); }

bool filledAbove(int x, int y, const void *k) {
  return rawAbove(x, y, k) || !smoothPatch(rawBelow, k, x, y);
}

bool rawWaterAt(int x, int y, const void *) {
  return memo(x, y, slot::rawWater, [&] {
    Structure s;
    BiomeId b = biomeAt(x, y);
    int level = heightAt(x, y);
    return !structureAt(x, y, s, 1) && !BIOMES[b]->fixedGround(x, y) &&
           (level ? tarn(x, y, level)
                  : lake(x, y, -std::max(spawnDry(x, y), structureDry(x, y))));
  });
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

int block(int v) { return (v - (v < 0 ? 2 : 0)) / 3; }

} // namespace

bool lake(int x, int y, float margin) {
  GroundBlend g = groundBlend(x, y);
  float bias = 0;
  for (int i = 0; i < g.count; i++)
    bias += LAKES[g.biome[i]] * g.weight[i];
  return noise_fbm(x * 0.011f + 41.7f, y * 0.011f - 13.3f, 3, 1201) <
         -0.3f + bias + margin;
}

bool smoothPatch(PatchTest test, const void *ctx, int x, int y) {
  if (solidCore(test, ctx, x, y))
    return true;
  int n = solidCore(test, ctx, x, y - 1) + solidCore(test, ctx, x, y + 1) +
          solidCore(test, ctx, x - 1, y) + solidCore(test, ctx, x + 1, y);
  return n >= 3;
}

bool isDirt(int x, int y) {
  if (active != Layer::Surface)
    return under::isDirt(x, y);
  return memo(x, y, slot::dirt, [&] {
    Structure s;
    return structureAt(x, y, s, 1) ? structureDirt(&s, x - s.x, y - s.y)
                                   : smoothPatch(rawDirtAt, nullptr, x, y);
  });
}

bool isWater(int x, int y) {
  if (active != Layer::Surface)
    return under::isWater(x, y);
  return memo(x, y, slot::water,
              [&] { return smoothPatch(rawWaterAt, nullptr, x, y); });
}

bool isIce(int x, int y) {
  return isWater(x, y) && BIOMES[biomeAt(x, y)]->winter();
}

int heightAt(int x, int y) {
  if (active != Layer::Surface)
    return under::heightAt(x, y);
  return int(memo(x, y, slot::level, [&] {
    int level = 0;
    for (int k = 1; k <= 3 && smoothPatch(filledAbove, &k, x, y); k++)
      level = k;
    return level;
  }));
}

bool isRamp(int x, int y) {
  if (active != Layer::Surface)
    return under::isRamp(x, y);
  int h = heightAt(x, y), b = block(x) * 3;
  if (hash2(block(x) * 13 + 7, y * 29 - 3) % 3 == 2)
    return false;
  for (int c = b; c < b + 3; c++)
    if (heightAt(c, y) != h || heightAt(c, y + 1) >= h || heightAt(c, y - 1) < h)
      return false;
  return true;
}

} // namespace world
