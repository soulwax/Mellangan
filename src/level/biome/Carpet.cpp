#include "level/biome/Carpet.hpp"

#include "level/World.hpp"

namespace {

bool own(int x, int y, BiomeId id) { return world::biomeAt(x, y) == id; }

struct Probe {
  bool (*test)(int, int, BiomeId);
  BiomeId id;
};

bool probe(int x, int y, const void *ctx) {
  const Probe *p = static_cast<const Probe *>(ctx);
  return p->test(x, y, p->id);
}

} // namespace

bool blockOf(bool (*test)(int, int, BiomeId), BiomeId id, int x, int y) {
  Probe p{test, id};
  return world::smoothPatch(probe, &p, x, y);
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
