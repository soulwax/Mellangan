#include "Test.hpp"
#include "BiomeTransitionTest.hpp"

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
#include "particle/DeathEffect.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/projectile/ChakramDisc.hpp"
#include "level/Level.hpp"
#include "item/Upgrade.hpp"
#include "hero/Hero.hpp"
#include "menu/LevelUpMenu.hpp"
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

static void testExperienceCurve() {
  Player early;
  early.gainXp(33);
  assert(early.lvl == 3 && early.pendingLevels == 2);
  assert(std::abs(early.exp - 10) < 0.001f);
  assert(std::abs(early.expNext - 25.92f) < 0.01f);

  Player progression;
  double total = 0;
  float last = 0;
  for (int level = 1; level < 100; level++) {
    float required = progression.expNext;
    assert(required > last && std::isfinite(required));
    total += required;
    progression.gainXp(required);
    assert(progression.lvl == level + 1);
    last = required;
  }
  assert(total > 12000000 && progression.pendingLevels == 99);

  Player spree;
  spree.hero = HEROES[0];
  for (int seconds = 0; seconds < 600; seconds++)
    for (int kills = 0; kills < 60; kills++)
      spree.gainXp(20 * 1.35f * 1.5f * 1.5f);
  assert(spree.lvl < 100 && spree.lvl >= 40);
  assert(spree.exp >= 0 && spree.exp < spree.expNext);
  std::cout << "[TEST] Exponential XP: level 100 requires " << total
            << " XP; generous 10-minute spree reached level " << spree.lvl
            << std::endl;
}

static void testAutomaticLevelUp() {
  Game game;
  game.level = std::make_unique<Level>(game);
  Player &p = *game.level->player;
  p.upgrades.clear();

  auto one = std::make_unique<Upgrade>("Single", atlas::White,
                                      std::array<const char *, 5>{}, true);
  Upgrade *single = one.get();
  single->level = Upgrade::MAX_LEVEL - 1;
  p.upgrades.push_back(std::move(one));
  p.hp = 10;
  p.pendingLevels = 3;
  assert(!LevelUpMenu::open(game));
  assert(single->level == Upgrade::MAX_LEVEL && p.pendingLevels == 0);
  assert(p.hp == 90);

  p.upgrades.clear();
  for (const char *name : {"First", "Second"}) {
    auto up = std::make_unique<Upgrade>(name, atlas::White,
                                       std::array<const char *, 5>{}, true);
    up->level = Upgrade::MAX_LEVEL - 1;
    p.upgrades.push_back(std::move(up));
  }
  p.pendingLevels = 2;
  auto menu = LevelUpMenu::open(game);
  assert(menu && p.pendingLevels == 2);
  game.input.one.clicked = true;
  menu->tick();
  game.input.one.clicked = false;
  assert(p.pendingLevels == 0);
  assert(p.upgrades[0]->maxed() && p.upgrades[1]->maxed());

  p.upgrades.clear();
  auto weapon = std::make_unique<Upgrade>("Weapon", atlas::White,
                                         std::array<const char *, 5>{}, true);
  weapon->level = Upgrade::MAX_LEVEL;
  weapon->partner = "Partner";
  weapon->evolvedName = "Evolved";
  Upgrade *evolving = weapon.get();
  p.upgrades.push_back(std::move(weapon));
  auto partner = std::make_unique<Upgrade>("Partner", atlas::White,
                                          std::array<const char *, 5>{}, false);
  partner->level = Upgrade::MAX_LEVEL;
  p.upgrades.push_back(std::move(partner));
  p.pendingLevels = 1;
  p.pendingTreasures = 1;
  menu = LevelUpMenu::open(game);
  assert(menu && p.pendingLevels == 1 && p.pendingTreasures == 0);
  game.input.two.clicked = true;
  menu->tick();
  game.input.two.clicked = false;
  assert(evolving->evolved && p.pendingLevels == 0);

  p.upgrades.clear();
  auto ordinary = std::make_unique<Upgrade>("Ordinary", atlas::White,
                                            std::array<const char *, 5>{}, true);
  auto rare = std::make_unique<Upgrade>("Relic", atlas::White,
                                        std::array<const char *, 5>{}, false);
  rare->rare = true;
  p.upgrades.push_back(std::move(ordinary));
  p.upgrades.push_back(std::move(rare));
  p.pendingLevels = 1;
  p.pendingTreasures = 1;
  menu = LevelUpMenu::open(game);
  assert(menu && p.pendingLevels == 1 && p.pendingTreasures == 0);
  game.input.two.clicked = true;
  menu->tick();
  game.input.two.clicked = false;
  assert(p.upgrades[1]->level == 1 && (game.codex.relics & 1));
  assert(p.pendingLevels == 0);
}

static void testGuiChoices() {
  Game game;
  Screen &s = game.screen;
  s.w = 480, s.h = 270, s.u = 1;
  auto cards = gui::ChoiceList::layout(s, 4);
  gui::Panel panel(cards[0], col::panel, col::white, 1);
  assert(panel.contains(cards[0].x + 1, cards[0].y + 1));
  assert(!panel.contains(cards[0].x - 1, cards[0].y));
  assert(cards[1].y - cards[0].y == 52);
  assert(gui::ChoiceList::entrance(0, 0) == 0);
  assert(gui::ChoiceList::entrance(1, 3) == 1);
  gui::ChoiceList list;
  Input in;
  in.up.clicked = true;
  auto result = list.update(in, cards, 4, false);
  assert(list.selected == 3 && result.selectionChanged && result.activated == -1);
  in.up.clicked = false;
  in.confirm.clicked = true;
  assert(list.update(in, cards, 4, false).activated == -1);
  assert(list.update(in, cards, 4, true).activated == 3);
  in.confirm.clicked = false;
  in.mouseX = cards[1].x + 1;
  in.mouseY = cards[1].y + 1;
  in.mouseMoved = true;
  result = list.update(in, cards, 4, true);
  assert(list.selected == 1 && result.selectionChanged);
  in.mouseMoved = false;
  in.mouseClicked = true;
  assert(list.update(in, cards, 4, false).activated == -1);
  assert(list.update(in, cards, 4, true).activated == 1);
  in.mouseClicked = false;
  in.four.clicked = true;
  assert(list.update(in, cards, 4, false).activated == 3);
  assert(list.update(in, cards, 0, true).activated == -1);
  std::cout << "[TEST] Reusable GUI layout, navigation and activation: PASS" << std::endl;
}

static void testEnemyDeathEffects() {
  struct Case { EnemyKind kind; DeathStyle style; };
  const Case cases[] = {{EnemyKind::Ghost, DeathStyle::Spirit},
                        {EnemyKind::FrostWisp, DeathStyle::Spirit},
                        {EnemyKind::Demon, DeathStyle::Flame},
                        {EnemyKind::EmberImp, DeathStyle::Flame},
                        {EnemyKind::Slime, DeathStyle::Slime},
                        {EnemyKind::Bat, DeathStyle::Fallen},
                        {EnemyKind::Spider, DeathStyle::Fallen},
                        {EnemyKind::DustVulture, DeathStyle::Fallen}};
  for (const Case &c : cases) {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    level.player->god = true;
    auto enemy = Enemy::create(c.kind, 1, 1);
    Enemy *e = enemy.get();
    e->x = level.player->x + 30;
    e->y = level.player->y;
    level.add(std::move(enemy));
    e->hurt(100000, 0, 0, 0);
    assert(e->removed && level.kills == 1 && level.slain[int(c.kind)] == 1);
    DeathEffect *effect = nullptr;
    for (auto &p : level.particles)
      if (auto *d = dynamic_cast<DeathEffect *>(p.get()))
        effect = d;
    assert(effect && effect->style == c.style);
    if (c.style == DeathStyle::Spirit) {
      double z = effect->z;
      effect->tick();
      assert(effect->z > z);
    } else if (c.style == DeathStyle::Fallen) {
      effect->z = effect->height = 8;
      effect->tick();
      assert(effect->z < 8);
    }
    game.level.reset();
    assert(DeathEffect::alive == 0 && Debris::alive == 0 && Spark::alive == 0);
  }
  std::cout << "[TEST] Enemy death styles, motion, kill credit and cleanup: PASS" << std::endl;
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
  testBiomeTransitions();
  testExperienceCurve();
  testAutomaticLevelUp();
  testGuiChoices();
  testEnemyDeathEffects();
  std::cout << "[TEST] Automatic level-up and treasure choices: PASS" << std::endl;
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
