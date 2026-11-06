#undef NDEBUG
#include <cassert>

#include "test/Tests.hpp"

#include <SDL3/SDL.h>

#include <atomic>
#include <iostream>
#include <vector>

#include "Workers.hpp"

namespace {
struct Round {
  std::vector<std::atomic<int>> hits;
  int round;
  explicit Round(int n, int r) : hits(n), round(r) {}
};

void mark(void *ctx, int begin, int end) {
  Round &r = *static_cast<Round *>(ctx);
  for (int i = begin; i < end; i++)
    r.hits[i].fetch_add(1 + r.round % 3);
}
} // namespace

void testWorkers() {
  Workers &w = Workers::shared();
  uint64_t start = SDL_GetPerformanceCounter();
  int rounds = 0;
  for (int i = 0; i < 4000; i++) {
    int n = 1 + i * 37 % 900, chunk = 1 + i % 64;
    Round r(n, i);
    w.run(n, chunk, mark, &r);
    for (auto &h : r.hits)
      assert(h.load() == 1 + i % 3);
    rounds++;
  }
  double ms = double(SDL_GetPerformanceCounter() - start) * 1000 / double(SDL_GetPerformanceFrequency());
  std::cout << "[TEST] Workers: " << rounds << " back-to-back rounds on " << w.size()
            << " threads cover every index exactly once, late workers never run a stale job ("
            << int(ms) << " ms): PASS" << std::endl;
}
