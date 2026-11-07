#include "level/world/Internal.hpp"

namespace world {

namespace {

const int LANDMARKS[BIOME_COUNT] = {0, 0, 5, 7, 2, 9, 0, 5, 6, 0, 2, 8, 4,
                                    5, 8, 9, 6, 6, 7, 3, 8, 0, 3, 4, 4, 1};
constexpr int LANDMARK_KINDS = 10;
const int MASONRY[BIOME_COUNT] = {0, 0, 0, 0, 3, 2, 0, 3, 1, 3, 1, 4, 0,
                                  3, 1, 2, 1, 3, 0, 3, 4, 0, 0, 1, 4, 4};

int floorDiv(int v, int d) { return (v - (v < 0 ? d - 1 : 0)) / d; }

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

bool rollStructure(int rx, int ry, Structure *out) {
  uint32_t h = hash2(rx * 31 + 7, ry * 17 - 3);
  int cx = rx * REGION + REGION / 2, cy = ry * REGION + REGION / 2;
  BiomeId id = biomeAt(cx, cy);
  const Biome *b = BIOMES[id];
  int realmX = floorDiv(rx, 4), realmY = floorDiv(ry, 4);
  uint32_t realm = hash2(realmX * 91 + 5, realmY * 47 - 11);
  bool landmark = rx == realmX * 4 + int(realm % 4) &&
                  ry == realmY * 4 + int(realm >> 2 & 3) &&
                  (realm >> 8) % 100 < 75 &&
                  std::abs(realmX) + std::abs(realmY) > 1;
  if ((!landmark && int(h % 100) >= b->structureChance) || lake(cx, cy, 0.0f))
    return false;

  StructureKind kind =
      landmark ? StructureKind(int(StructureKind::AncientTree) +
                               (realm >> 16 & 1 ? LANDMARKS[id]
                                                : int((realm >> 20) % LANDMARK_KINDS)))
               : pickKind(b, h >> 8);
  int w, ht;
  STRUCTURES[int(kind)].size(h, &w, &ht);
  int spanX = REGION - 4 - w, spanY = REGION - 4 - ht;
  auto level = [&](int x, int y) {
    int ground = heightAt(x + w / 2, y + ht / 2);
    for (int yy = y - 2; yy <= y + ht + 1; yy++)
      for (int xx = x - 2; xx <= x + w + 1; xx++)
        if (heightAt(xx, yy) != ground)
          return false;
    return true;
  };
  int x = 0, y = 0, tries = 0;
  for (uint32_t pick = h; tries < 4; tries++, pick = hash2(int(pick), tries)) {
    x = rx * REGION + 2 + int((pick >> 18) % uint32_t(spanX + 1));
    y = ry * REGION + 2 + int((pick >> 24) % uint32_t(spanY + 1));
    if (level(x, y))
      break;
  }
  if (tries == 4)
    return false;
  if (x - 1 < 6 && x + w + 1 > -6 && y - 1 < 6 && y + ht + 1 > -6)
    return false;

  *out = {kind, x, y, w, ht, rx, ry, hash2(int(h), rx ^ (ry << 8)), MASONRY[id]};
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
  int by = 0, bx = 0;
  Structure s;
  auto visit = [&](int xr, int yr) {
    if (!structureIn(xr, yr, s) || s.contains(tx, ty) || (keep && !keep(&s, ctx)))
      return;
    double xd = s.centerX() - x, yd = s.centerY() - y, d = xd * xd + yd * yd;
    if (d < best || (d == best && (yr < by || (yr == by && xr < bx))))
      best = d, by = yr, bx = xr, out = s;
  };
  for (int k = 0; k <= maxRegions; k++) {
    double gap = double(k - 1) * REGION * 16;
    if (k > 1 && gap * gap > best)
      break;
    for (int i = -k; i <= k; i++) {
      visit(rx + i, ry - k);
      if (k)
        visit(rx + i, ry + k);
    }
    for (int i = 1 - k; i < k; i++)
      visit(rx - k, ry + i), visit(rx + k, ry + i);
  }
  return best < INFINITY;
}

} // namespace world
