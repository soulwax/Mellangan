#include "level/world/Internal.hpp"

namespace world {

namespace {

TileMemo tiles[128 * 64];

struct BlendNode {
  int x, y;
  uint32_t epoch;
  GroundBlend blend;
};
BlendNode blendMemo[64 * 64];

int floor2(int v) { return v / 2 - (v < 0 && v % 2 != 0); }

GroundBlend compactBlend(const float *weights) {
  GroundBlend out{};
  float total = 0;
  for (int i = 0; i < BIOME_COUNT; i++)
    total += weights[i];
  for (int n = 0; n < 3; n++) {
    int best = -1;
    for (int i = 0; i < BIOME_COUNT; i++) {
      bool used = false;
      for (int k = 0; k < out.count; k++)
        used |= int(out.biome[k]) == i;
      if (!used && (best < 0 || weights[i] > weights[best]))
        best = i;
    }
    if (best < 0 || weights[best] <= 0)
      break;
    out.biome[out.count] = BiomeId(best);
    out.weight[out.count++] = weights[best] / total;
  }
  float sum = 0;
  for (int i = 0; i < out.count; i++)
    sum += out.weight[i];
  if (sum > 0)
    for (int i = 0; i < out.count; i++)
      out.weight[i] /= sum;
  return out;
}

const GroundBlend &blendNode(int x, int y) {
  BlendNode *node = &blendMemo[(x / 2 & 63) | (y / 2 & 63) << 6];
  if (node->epoch != noise_epoch || node->x != x || node->y != y) {
    constexpr int K[5] = {1, 2, 3, 2, 1};
    float totals[BIOME_COUNT]{};
    for (int iy = -2; iy <= 2; iy++)
      for (int ix = -2; ix <= 2; ix++)
        totals[biomeAt(x + ix * 2, y + iy * 2)] += float(K[ix + 2] * K[iy + 2]);
    node->x = x;
    node->y = y;
    node->epoch = noise_epoch;
    node->blend = compactBlend(totals);
  }
  return node->blend;
}

GroundBlend interpolateBlend(const GroundBlend &a, const GroundBlend &b,
                             const GroundBlend &c, const GroundBlend &d,
                             float fx, float fy) {
  float totals[BIOME_COUNT]{};
  auto add = [&](const GroundBlend &blend, float amount) {
    for (int i = 0; i < blend.count; i++)
      totals[blend.biome[i]] += blend.weight[i] * amount;
  };
  add(a, (1 - fx) * (1 - fy));
  add(b, fx * (1 - fy));
  add(c, (1 - fx) * fy);
  add(d, fx * fy);
  return compactBlend(totals);
}

BiomeId rawBiome(int x, int y) {
  return BiomeId(memo(x, y, slot::raw, [&] { return biomeForLocation(x, y); }));
}

} // namespace

TileMemo *memoAt(int x, int y) {
  TileMemo *m = &tiles[(x & 127) | (y & 63) << 7];
  if (m->epoch != noise_epoch || m->x != x || m->y != y)
    m->x = x, m->y = y, m->bits = 0, m->epoch = noise_epoch;
  return m;
}

BiomeId biomeAt(int x, int y) {
  return BiomeId(memo(x, y, slot::biome, [&] {
    uint8_t votes[BIOME_COUNT]{};
    BiomeId own = rawBiome(x, y), best = own;
    for (int dy = -1; dy <= 1; dy++)
      for (int dx = -1; dx <= 1; dx++) {
        BiomeId b = rawBiome(x + dx, y + dy);
        if (++votes[b] > votes[best] || (b == own && votes[b] == votes[best]))
          best = b;
      }
    return best;
  }));
}

GroundBlend groundBlend(int x, int y) {
  int gx = floor2(x), gy = floor2(y), x0 = gx * 2, y0 = gy * 2;
  float fx = (x - x0) * 0.5f, fy = (y - y0) * 0.5f;
  fx = fx * fx * (3 - 2 * fx);
  fy = fy * fy * (3 - 2 * fy);
  return interpolateBlend(blendNode(x0, y0), blendNode(x0 + 2, y0),
                          blendNode(x0, y0 + 2), blendNode(x0 + 2, y0 + 2),
                          fx, fy);
}

const Biome *blendPick(int x, int y, uint32_t roll) {
  GroundBlend blend = groundBlend(x, y);
  float r = float(roll % 10000) / 10000.0f;
  for (int i = 0; i < blend.count; i++) {
    if (r < blend.weight[i])
      return BIOMES[blend.biome[i]];
    r -= blend.weight[i];
  }
  return BIOMES[biomeAt(x, y)];
}

const Biome *blendedBiome(int x, int y) {
  const Biome *here = BIOMES[biomeAt(x, y)];
  const Biome *there = blendPick(x, y, hash2(x * 31 + 5, y * 17 + 9));
  return there->winter() == here->winter() ? there : here;
}

bool findBiome(int biome, int &x, int &y, int reach) {
  for (int r = 0; r < reach; r += 12)
    for (int i = 0, n = std::max(1, r / 2); i < n; i++) {
      double a = TAU * i / n;
      int tx = int(std::cos(a) * r), ty = int(std::sin(a) * r);
      bool deep = true;
      for (int k = 0; k < 4; k++)
        deep &= biomeAt(tx + (k & 1 ? 6 : -6), ty + (k & 2 ? 6 : -6)) == biome;
      if (deep && biomeAt(tx, ty) == biome)
        return x = tx, y = ty, true;
    }
  return false;
}

} // namespace world
