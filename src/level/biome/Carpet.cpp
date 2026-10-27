#include "level/biome/Carpet.hpp"

namespace {

bool own(int x, int y, BiomeId id) { return world::biomeAt(x, y) == id; }

} // namespace

bool blockOf(bool (*test)(int, int, BiomeId), BiomeId id, int x, int y) {
  if (!test(x, y, id))
    return false;
  for (int dy = -1; dy <= 0; dy++)
    for (int dx = -1; dx <= 0; dx++)
      if (test(x + dx, y + dy, id) && test(x + dx + 1, y + dy, id) &&
          test(x + dx, y + dy + 1, id) && test(x + dx + 1, y + dy + 1, id))
        return true;
  return false;
}

bool Carpet::covered(int x, int y) const { return blockOf(own, self, x, y); }

Sprite Carpet::edge(int set, int mask) {
  int o = world::rim(mask);
  return atlas::carpet(set, 3 + o / 12 * 3 + o % 12);
}

bool Carpet::ground(int x, int y, Sprite *out) const {
  if (!covered(x, y))
    return false;
  int mask = int(covered(x, y - 1)) | covered(x, y + 1) << 1 |
             covered(x - 1, y) << 2 | covered(x + 1, y) << 3;
  if (mask != 15) {
    *out = edge(set, mask);
    return true;
  }
  int r = hash2(x, y) % 100;
  *out = atlas::carpet(set, r < 12 ? 1 : r < 16 ? 2 : 0);
  return true;
}
