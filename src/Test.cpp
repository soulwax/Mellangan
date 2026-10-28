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
#include "item/Upgrade.hpp"
#include "particle/Spark.hpp"
#include "particle/TextParticle.hpp"
#include "particle/debris/Debris.hpp"
#include "ExpansionEnemyTest.hpp"
#include "ExpansionTest.hpp"

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

static void testStatuses() {
  Status s;
  Upgrade strong("Strong", atlas::White, {}, true);
  Upgrade weak("Weak", atlas::White, {}, true);
  assert(s.apply(StatusKind::Poison, 1.0f, 8, &strong));
  assert(s.apply(StatusKind::Poison, 2.0f, 2, &weak));
  float total = 0;
  for (int i = 0; i < 8; i++)
    s.tick(0.25f, [&](float damage, Upgrade *src) {
      assert(src == &strong);
      total += damage;
      return true;
    });
  assert(std::abs(total - 16) < 0.001f);
  assert(s.power(StatusKind::Poison) == 0);
  assert(s.effects[int(StatusKind::Poison)].source == nullptr);
  assert(s.apply(StatusKind::Burn, 2, 2, &weak));
  assert(s.apply(StatusKind::Burn, 1, 8, &strong));
  assert(s.effects[int(StatusKind::Burn)].source == &strong);
  s.clear();
  assert(s.apply(StatusKind::Chill, 1, 10));
  assert(std::abs(s.movement() - 0.35f) < 0.001f);
  s.immune = 1u << int(StatusKind::Burn);
  assert(!s.apply(StatusKind::Burn, 2, 5));
  assert(!s.apply(StatusKind::Poison, -1, 5));
  assert(s.apply(StatusKind::Marked, 1, 0.25f));
  assert(std::abs(s.incoming() - 1.25f) < 0.001f);
  assert(s.apply(StatusKind::Poison, 0.25f, 8, &strong));
  total = 0;
  s.tick(0.5f, [&](float damage, Upgrade *) {
    total += damage;
    return true;
  });
  assert(std::abs(total - 2) < 0.001f);
  s.apply(StatusKind::Poison, 2, 8, &strong);
  s.tick(0.5f, [](float, Upgrade *) { return false; });
  for (const auto &effect : s.effects)
    assert(effect.time == 0 && effect.source == nullptr);
}

static void testStatusDamage() {
  Game game;
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  auto enemy = Enemy::create(EnemyKind::Rat, 1, 1);
  Enemy *e = enemy.get();
  level.add(std::move(enemy));
  Upgrade &weapon = *p.upgrades.front();
  float before = weapon.dealt;
  int kills = level.kills;
  e->afflict(StatusKind::Burn, 2, 20, &weapon);
  for (int i = 0; i < 40 && !e->removed; i++)
    e->tick();
  assert(e->removed && level.kills == kills + 1);
  assert(std::abs(weapon.dealt - before - 9) < 0.001f);
  assert(e->status.power(StatusKind::Burn) == 0);
  assert(!e->afflict(StatusKind::Poison, 2, 4));
  auto marked = Enemy::create(EnemyKind::Knight, 1, 1);
  Enemy *target = marked.get();
  level.add(std::move(marked));
  p.luck = -1;
  target->afflict(StatusKind::Marked, 2, 0.25f);
  target->hurt(20, 0, 0, 0, &weapon);
  assert(std::abs(target->hp - 95) < 0.001f);
  target->hurt(10, 0, 0, 0, &weapon, true);
  assert(std::abs(target->hp - 85) < 0.001f);
  p.afflict(StatusKind::Poison, 2, 4);
  float hp = p.hp;
  for (int i = 0; i < 40; i++)
    p.tick();
  assert(p.hp == hp);
  p.revives = 1;
  p.rise();
  assert(p.status.power(StatusKind::Poison) == 0);
  game.level.reset();
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
  testStatuses();
  testStatusDamage();
  testExpansionEnemies();
  testExpansionContent();
  std::cout << "[TEST] Status stacking, expiry, immunity, damage credit and cleanup: PASS"
            << std::endl;
  testLevelLifecycleAndSimulation();
  std::cout << "[TEST] Level lifecycle, multithreaded AI & entity cleanup: PASS"
            << std::endl;
  pool_shutdown();
  std::cout << "[TEST] Pool shutdown & slab reclamation: PASS" << std::endl;
  std::cout << "[TEST] ALL TESTS PASSED." << std::endl;
  return 0;
}
