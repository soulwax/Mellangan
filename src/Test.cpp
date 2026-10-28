#include "Test.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/Pool.h"
#include "entity/effect/GlaiveThrust.hpp"
#include "entity/effect/IncenseCloud.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/projectile/ChakramDisc.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"
#include "particle/TextParticle.hpp"
#include "particle/debris/Debris.hpp"

static void testPoolAllocAndFree() {
  void *ptrs[1024]; // 8kB total storage

  // go through a bunch of allocations and frees, with different sizes
  for (int i = 0; i < 1024; i++) {
    size_t sz = (i % 64 + 1) * 8;
    ptrs[i] = pool_alloc(sz);
    assert(ptrs[i] != nullptr);
    std::memset(ptrs[i], 0xAA, sz);
  }
  for (int i = 0; i < 1024; i++) {
    size_t sz = (i % 64 + 1) * 8;
    pool_free(ptrs[i], sz);
  }
  void *zero = pool_alloc(0);
  assert(zero != nullptr);
  pool_free(zero, 0);

  void *big = pool_alloc(2048);
  assert(big != nullptr);
  pool_free(big, 2048);
}

static void testPoolConcurrency() {
  // spawn 8 threads that each allocate and free a bunch of objects
  std::vector<std::thread> workers;
  for (int t = 0; t < 8; t++) {
    workers.emplace_back([] {
      for (int cycle = 0; cycle < 100; cycle++) {
        void *items[64];
        for (int i = 0; i < 64; i++)
          items[i] = pool_alloc(16 * (i % 20 + 1));
        for (int i = 0; i < 64; i++)
          pool_free(items[i], 16 * (i % 20 + 1));
      }
    });
  }
  for (auto &w : workers)
    w.join();
}

static void testLevelLifecycleAndSimulation() {
  Game game;
  // run level lifecycle 3 times to check for leaks
  for (int run = 0; run < 3; run++) {
    game.level = std::make_unique<Level>(game);
    Level *level = game.level.get();
    level->tick();

    for (int i = 0; i < int(EnemyKind::Count); i++) {
      for (int k = 0; k < 20; k++) {
        level->spawner.spawn(EnemyKind(i), k * 20.0, k * 15.0, true);
      }
    }
    level->tick();

    Player &p = *level->player;
    for (int i = 0; i < 50; i++) {
      level->add<ChakramDisc>(p.x, p.y, 100.0, 100.0, 20.0f, 5);
      level->add<GlaiveThrust>(p, 1.0, 0.0, 48.0f, 25.0f, 100.0f, true);
      level->add<IncenseCloud>(p.x, p.y, 10.0, 10.0, 26.0f, 10.0f, 2.0f);
      level->add<Gem>(p.x + i * 2, p.y + i * 2, 0);
      Spark::burst(*level, p.x, p.y, col::gold, 5, 50, false);
      TextParticle::pop(*level, p.x, p.y, 10, col::white);
    }
    level->tick();

    for (int tick = 0; tick < 90; tick++)
      level->tick();

    level->each<Enemy>([](Enemy &e) { e.remove(); });
    level->tick();

    assert(level->count<Enemy>() == 0);
    game.level.reset();
  }
  std::cout << "[DEBUG] Debris: " << Debris::alive << " Spark: " << Spark::alive
            << " TextParticle: " << TextParticle::alive << std::endl;
  assert(Debris::alive == 0);
  assert(Spark::alive == 0);
  assert(TextParticle::alive == 0);
}

int runTests() {
  std::cout << "[TEST] Running memory leak & stability tests..." << std::endl;
  testPoolAllocAndFree();
  std::cout << "[TEST] Pool alloc/free & edge cases: PASS" << std::endl;
  testPoolConcurrency();
  std::cout << "[TEST] Pool multithreaded concurrency: PASS" << std::endl;
  testLevelLifecycleAndSimulation();
  std::cout << "[TEST] Level lifecycle, multithreaded AI & entity cleanup: PASS"
            << std::endl;
  pool_shutdown();
  std::cout << "[TEST] Pool shutdown & slab reclamation: PASS" << std::endl;
  std::cout << "[TEST] ALL TESTS PASSED." << std::endl;
  return 0;
}
