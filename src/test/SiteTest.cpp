#undef NDEBUG
#include <cassert>

#include "test/Tests.hpp"

#include <SDL3/SDL.h>

#include <cmath>
#include <iostream>

#include "level/World.hpp"

namespace {
bool shrine(const Structure *s, const void *) { return s->kind == StructureKind::Shrine; }

bool scan(double x, double y, int reach, Structure &out, world::StructureFilter keep) {
  int tx = int(std::floor(x / 16)), ty = int(std::floor(y / 16)),
      rx = world::regionOf(tx), ry = world::regionOf(ty);
  double best = INFINITY;
  Structure s;
  for (int yr = ry - reach; yr <= ry + reach; yr++)
    for (int xr = rx - reach; xr <= rx + reach; xr++) {
      if (!world::structureIn(xr, yr, s) || s.contains(tx, ty) || (keep && !keep(&s, nullptr)))
        continue;
      double xd = s.centerX() - x, yd = s.centerY() - y, d = xd * xd + yd * yd;
      if (d < best)
        best = d, out = s;
    }
  return best < INFINITY;
}

double ms(uint64_t from) {
  return double(SDL_GetPerformanceCounter() - from) * 1000 / double(SDL_GetPerformanceFrequency());
}
} // namespace

void testNearestStructure() {
  double ring = 0, full = 0, far = 0;
  for (int i = 0; i < 6; i++) {
    double x = (i * 7919 % 4001 - 2000) * 16.0 + 5, y = (i * 104729 % 3001 - 1500) * 16.0 + 9;
    for (int reach : {3, 8})
      for (world::StructureFilter keep : {world::StructureFilter(nullptr), shrine}) {
        Structure a, b;
        uint64_t t = SDL_GetPerformanceCounter();
        bool fa = world::nearestStructure(x, y, reach, a, keep, nullptr);
        ring += ms(t), t = SDL_GetPerformanceCounter();
        bool fb = scan(x, y, reach, b, keep);
        full += ms(t);
        assert(fa == fb && (!fa || (a.rx == b.rx && a.ry == b.ry && a.kind == b.kind)));
      }
    Structure s;
    uint64_t t = SDL_GetPerformanceCounter();
    assert(world::nearestStructure(x + 64000, y - 64000, 40, s, shrine, nullptr));
    far += ms(t);
  }
  std::cout << "[TEST] Nearest structure: rings stop at the first ring that cannot be closer and "
               "match the full scan ("
            << int(ring) << " ms against " << int(full) << " ms), a gate's 40-region search takes "
            << int(far * 1000 / 6) << " us: PASS" << std::endl;
}
