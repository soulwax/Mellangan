#undef NDEBUG
#include <cassert>
#ifdef _WIN32
#include <stdlib.h>
#endif

#include "Test.hpp"
#include "BiomeTransitionTest.hpp"
#include "BiomeEncounterTest.hpp"

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
#include "particle/VisualDetail.hpp"
#include "gfx/StatusVisual.hpp"
#include "entity/enemy/Enemy.hpp"
#include "item/skill/Skill.hpp"
#include "item/weapon/Recipe.hpp"
#include "item/talent/ExpansionTalents.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/Ability.hpp"
#include "level/Flavour.hpp"
#include "entity/enemy/Boss.hpp"
#include "entity/Trait.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/enemy/Renegade.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/projectile/Arrow.hpp"
#include "entity/projectile/Bomb.hpp"
#include "entity/effect/Sweep.hpp"
#include "entity/effect/Strike.hpp"
#include "entity/projectile/Slash.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/projectile/ChakramDisc.hpp"
#include "level/Level.hpp"
#include "level/Noise.h"
#include "level/World.hpp"
#include "level/Prop.hpp"
#include "level/Structure.hpp"
#include "item/Upgrade.hpp"
#include "hero/Hero.hpp"
#include "menu/LevelUpMenu.hpp"
#include "particle/Spark.hpp"
#include "particle/TextParticle.hpp"
#include "particle/debris/Debris.hpp"
#include "ExpansionEnemyTest.hpp"
#include "ExpansionTest.hpp"
#include "ArsenalTest.hpp"

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

static void testBalance() {
  const char *text = "; comment\n[all]\nxp = 2\nhorde_max = 10 ; trailing\n"
                     "[nightmare]\nxp = 3\n[hurt_me_plenty]\nplayer_hp = 120\n";
  Balance plenty, nightmare, uv;
  plenty.load(text, Difficulty::HurtMePlenty);
  nightmare.load(text, Difficulty::Nightmare);
  uv.load(text, Difficulty::UltraViolence);
  assert(plenty.xp == 2 && plenty.playerHp == 120 && plenty.hordeMax == 10);
  assert(nightmare.xp == 3 && nightmare.playerHp == 100);
  assert(uv.xp == 2 && uv.playerHp == 100);
  assert(difficultyFrom("nightmare") == Difficulty::Nightmare);
  assert(difficultyFrom("garbage") == Difficulty::HurtMePlenty);
  std::cout << "[TEST] Balance tiers, overrides and comments: PASS" << std::endl;
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

static void testCombatReadability() {
  std::vector<std::unique_ptr<Spark>> sparks;
  int shown = 0;
  for (int i = 0; i < 128; i++) {
    sparks.push_back(std::make_unique<Spark>(0, 0, 0, 0, col::white, 1, 1, false));
    bool visible = showParticle(*sparks.back(), true);
    assert(showParticle(*sparks.back(), false));
    assert(visible == showParticle(*sparks.back(), true));
    shown += visible;
  }
  assert(shown > 0 && shown < 100);
  TextParticle text(0, 0, 1, col::white);
  DeathEffect death(0, 0, 0, atlas::tile(atlas::t::Bat), 1, 1, col::white,
                    DeathStyle::Fallen);
  assert(showParticle(text, true) && showParticle(death, true));
  Status status;
  for (int i = 0; i < int(StatusKind::Count); i++) {
    auto kind = StatusKind(i);
    assert(status.apply(kind, 1, 0.2f));
    assert(statusName(kind)[0] && statusColor(kind).a == 1);
  }
  for (int i = 0; i < int(StatusKind::Count); i++)
    assert(status.power(StatusKind(i)) > 0);
  Input input;
  SDL_Event e{};
  e.type = SDL_EVENT_KEY_DOWN;
  e.key.scancode = SDL_SCANCODE_V;
  input.handle(e, 1);
  input.tick();
  assert(input.particles.clicked);
  e.type = SDL_EVENT_WINDOW_FOCUS_LOST;
  input.handle(e, 1);
  input.tick();
  assert(!input.particles.down);
  std::cout << "[TEST] Particle detail, simultaneous status indicators and input: PASS" << std::endl;
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

static void testSidedAbilities() {
  Game game;
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &p = *level.player;
  auto foe = Enemy::create(EnemyKind::Knight, 4, 1);
  Enemy *e = foe.get();
  e->x = p.x + 40, e->y = p.y;
  level.add(std::move(foe));
  level.tick();
  level.tick();

  int sides = 0;
  level.foesOf(*e, e->x, e->y, 80, [&](Mob &m) { sides += &m == &p ? 1 : 100; });
  level.foesOf(p, p.x, p.y, 80, [&](Mob &m) { sides += &m == e ? 10 : 1000; });
  assert(sides == 11);

  float hp = p.hp, foeHp = e->hp;
  Arrow *hostile = level.add<Arrow>(e->x - 6, e->bodyY(), -300, 0, 10, 1);
  hostile->hostile = true;
  for (int i = 0; i < 30 && !hostile->removed; i++)
    level.tick();
  assert(p.hp < hp && e->hp == foeHp);

  p.invuln = 0, hp = p.hp;
  Arrow *friendly = level.add<Arrow>(p.x + 6, p.bodyY(), 300, 0, 10, 1);
  for (int i = 0; i < 30 && !friendly->removed; i++)
    level.tick();
  assert(e->hp < foeHp && p.hp == hp);

  p.luck = -1;
  e->traits.add(Trait::Warded);
  foeHp = e->hp;
  e->hurt(8, 0, 0, 0);
  assert(std::abs(foeHp - e->hp - 2) < 1e-3f && e->traits.ward == 4);
  for (int i = 0; i < 4; i++)
    e->hurt(1, 0, 0, 0);
  foeHp = e->hp;
  e->hurt(8, 0, 0, 0);
  assert(e->traits.ward == 0 && std::abs(foeHp - e->hp - 8) < 1e-3f);

  e->traits.add(Trait::Splitting), e->traits.add(Trait::Volatile);
  size_t before = level.count<Enemy>();
  e->hurt(1e6f, 0, 0, 0);
  level.tick();
  bool blast = false;
  for (auto &particle : level.particles)
    blast |= dynamic_cast<Blast *>(particle.get()) != nullptr;
  assert(level.count<Enemy>() >= before + 1 && blast);
  game.level.reset();
  std::cout << "[TEST] Sided projectiles, foe lookup and traits on any mob: PASS" << std::endl;
}

static void testSidedEffects() {
  Game game;
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &p = *level.player;
  auto foe = Enemy::create(EnemyKind::Knight, 4, 1);
  Enemy *e = foe.get();
  e->x = p.x + 30, e->y = p.y;
  level.add(std::move(foe));
  level.tick();
  level.tick();

  float hp = p.hp;
  Sweep *sweep = level.add<Sweep>(*e, 30, 10);
  for (int i = 0; i < 40 && !sweep->removed; i++)
    level.tick();
  assert(p.hp < hp);

  p.invuln = 0, hp = p.hp;
  Bomb *bomb = level.add<Bomb>(e->x, e->bodyY(), p.x, p.y, 10, 20, 1, 0.5f);
  bomb->hostile = true;
  for (int i = 0; i < 60 && !bomb->removed; i++)
    level.tick();
  assert(p.hp < hp);

  p.invuln = 0, hp = p.hp;
  Slash *slash = level.add<Slash>(*e, -1, 10, 1.2f);
  for (int i = 0; i < 20 && !slash->removed; i++)
    level.tick();
  assert(p.hp < hp);

  p.invuln = 0, hp = p.hp;
  float foeHp = e->hp;
  level.add<Strike>(p.x, p.y, 10, 24)->hostile = true;
  level.tick();
  level.tick();
  assert(p.hp < hp && e->hp == foeHp);

  Sweep *orphan = level.add<Sweep>(*e, 30, 10);
  e->hurt(1e6f, 0, 0, 0);
  for (int i = 0; i < 3; i++)
    level.tick();
  assert(orphan->removed);
  game.level.reset();
  std::cout << "[TEST] Sided sweeps, slashes, strikes, bombs and orphaned effects: PASS" << std::endl;
}

static void testFoeKits() {
  const char *const KITS[] = {"Arcane Staff", "Orbiting Knives", "Moon Blade",
                              "Storm Rod", "Frost Nova", "Chain Lightning"};
  for (const char *kit : KITS) {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    p.luck = -1;
    for (auto &up : p.upgrades)
      up->level = 0;
    auto make = [&](double dx) {
      auto foe = Enemy::create(EnemyKind::Knight, 50, 1);
      Enemy *e = foe.get();
      e->x = p.x + dx, e->y = p.y, e->aware = false;
      level.add(std::move(foe));
      return e;
    };
    Enemy *caster = make(36), *bystander = make(-36);
    level.tick();
    level.tick();
    Upgrade *up = caster->grant(kit, 5);
    assert(up && up->level > 0 && up->hostile() && up->holder == caster);
    float hp = p.hp, other = bystander->hp;
    for (int i = 0; i < 600 && p.hp >= hp; i++) {
      p.invuln = 0;
      caster->x = p.x + 36, caster->y = p.y;
      level.tick();
    }
    assert(p.hp < hp && bystander->hp == other);

    caster->hurt(1e9f, 0, 0, 0);
    for (int i = 0; i < 90; i++)
      level.tick();
    assert(!level.retired.empty() && level.retired.front()->holder == nullptr);
    game.level.reset();
  }
  std::cout << "[TEST] Weapons held by foes hit the player, spare foes, outlive their holder: PASS"
            << std::endl;
}

static void testArtsBothSides() {
  const char *const ARTS[] = {"ARROW STORM", "SHIELD CHARGE", "MIASMA",
                              "CHAIN STORM", "DEATH GRIP", "SACRED LIGHT",
                              "LUNGE", "BRAMBLES", "MINEFIELD", "COMET",
                              "POWDER KEG", "WAR CRY", "WHIRLWIND",
                              "GROUND SLAM", "FIRE RING", "SNARE NET",
                              "HUNTER'S MARK", "FLASK BARRAGE", "TEMPEST",
                              "BLOOD PACT", "HALLOWED GROUND", "FLASH STEP",
                              "OVERGROWTH"};
  for (const char *art : ARTS) {
    for (int side = 0; side < 2; side++) {
      Game game;
      game.level = std::make_unique<Level>(game);
      Level &level = *game.level;
      Player &p = *level.player;
      p.luck = -1, p.hp = p.maxHp = 1e6f;
      Skill *skill = nullptr;
      for (auto &up : p.upgrades)
        if (up->skill && std::strcmp(up->name, art) == 0)
          skill = static_cast<Skill *>(up.get());
      assert(skill);
      std::vector<Enemy *> foes;
      for (double dx : {40.0, 75.0, 110.0, -75.0}) {
        auto foe = Enemy::create(EnemyKind::Knight, 50, 1);
        foe->x = p.x + dx, foe->y = p.y, foe->aware = false;
        foes.push_back(foe.get());
        level.add(std::move(foe));
      }
      Enemy *caster = foes[1], *bystander = foes[3];
      double px = p.x, py = p.y;
      level.tick();
      level.tick();
      float hp = p.hp, other = bystander->hp, before = 0;
      for (Enemy *e : foes)
        before += e->hp;
      bool hit = false;
      for (int cast = 0; cast < 15 && !hit; cast++) {
        p.x = px, p.y = py, p.xAim = 1, p.yAim = 0;
        caster->x = px + 75, caster->y = py;
        skill->use(side ? static_cast<Mob &>(*caster) : p);
        for (int i = 0; i < 120 && !hit; i++) {
          p.invuln = 0;
          level.tick();
          float now = 0;
          for (Enemy *e : foes)
            now += e->removed ? 0 : e->hp;
          hit = side ? p.hp < hp : now < before;
        }
      }
      assert(hit);
      if (side)
        assert(bystander->hp == other);
      game.level.reset();
    }
  }
  std::cout << "[TEST] New class skills strike foes for the player and the player for foes: PASS"
            << std::endl;
}

static void testSpawnShore() {
  uint32_t salt = noise_salt;
  int longest = 0;
  for (uint32_t seed = 0; seed < 100; seed++) {
    noise_reseed(seed);
    for (int y = -5; y <= 5; y++)
      for (int x = -5; x <= 5; x++)
        assert(!world::isWater(x, y));
    for (int dir = 0; dir < 4; dir++) {
      int ox = dir == 0 ? 1 : dir == 1 ? -1 : 0, oy = dir == 2 ? 1 : dir == 3 ? -1 : 0;
      for (int a = -30; a <= 30; a++)
        for (int b = -30, run = 0; b <= 30; b++) {
          int x = ox ? a : b, y = ox ? b : a;
          run = !world::isWater(x, y) && world::isWater(x + ox, y + oy) ? run + 1 : 0;
          longest = std::max(longest, run);
        }
    }
  }
  int rims = 0;
  Prop prop;
  for (uint32_t seed = 0; seed < 100; seed++) {
    noise_reseed(seed);
    for (int y = -2; y <= 2; y++)
      for (int x = -2; x <= 2; x++)
        assert(x * x + y * y >= 7 || !world::propAt(x, y, prop));
    int ring = 0;
    for (int i = -4; i <= 4; i++)
      ring += world::propAt(i, -4, prop) + world::propAt(i, 4, prop) +
              (i > -4 && i < 4 ? world::propAt(-4, i, prop) + world::propAt(4, i, prop) : 0);
    rims += ring >= 10;
  }
  int sites = 0, straight = 0;
  for (uint32_t seed = 0; seed < 10; seed++) {
    noise_reseed(seed);
    Structure st;
    for (int ry = -6; ry < 6; ry++)
      for (int rx = -6; rx < 6; rx++) {
        bool found = false;
        for (int dy = 0; dy < world::REGION && !found; dy += 2)
          for (int dx = 0; dx < world::REGION && !found; dx += 2)
            found = world::structureAt(rx * world::REGION + dx, ry * world::REGION + dy, st, 0);
        if (!found)
          continue;
        if (st.kind == StructureKind::Ruin) {
          int dirt = 0;
          for (int y = st.y; y < st.y + st.h; y++)
            for (int x = st.x; x < st.x + st.w; x++)
              dirt += world::isDirt(x, y);
          assert(dirt > 4 && dirt * 10 < st.w * st.h * 9);
        }
        int wet = 0;
        for (int y = st.y - 4; y < st.y + st.h + 4; y++)
          for (int x = st.x - 4; x < st.x + st.w + 4; x++) {
            bool w = world::isWater(x, y);
            assert(!w || !st.contains(x, y, 1));
            wet += w;
          }
        if (!wet)
          continue;
        sites++;
        int run = 0, best = 0;
        for (int dir = 0; dir < 4; dir++) {
          int ox = dir == 0 ? 1 : dir == 1 ? -1 : 0, oy = dir == 2 ? 1 : dir == 3 ? -1 : 0;
          for (int a = -12; a <= 12; a++) {
            run = 0;
            for (int b = -12; b <= 12; b++) {
              int x = st.x + st.w / 2 + (ox ? a : b), y = st.y + st.h / 2 + (ox ? b : a);
              run = !world::isWater(x, y) && world::isWater(x + ox, y + oy) ? run + 1 : 0;
              best = std::max(best, run);
            }
          }
        }
        straight += best >= 8;
      }
  }
  noise_reseed(salt);
  assert(longest < 18 && rims <= 5 && sites > 20 && straight * 10 < sites * 3);
  std::cout << "[TEST] Shores and clearings stay organic: longest straight spawn coast "
            << longest << ", dense square clearing rims " << rims << "/100, straight structure shores "
            << straight << "/" << sites << ": PASS" << std::endl;
}

static void testHills() {
  uint32_t salt = noise_salt;
  int cliffs = 0, straight = 0, peaks = 0, ramps = 0, tarns = 0, land[2]{}, woods[2]{};
  Prop prop;
  for (uint32_t seed = 0; seed < 2; seed++) {
    noise_reseed(seed);
    for (int sx = -180; sx < 180; sx += 120)
      for (int y = -180; y < 180; y++)
        for (int x = sx, run = 0; x < sx + 120; x++) {
          int h = world::heightAt(x, y);
          auto at = [&](int dx, int dy) { return world::heightAt(x + dx, y + dy); };
          for (int k = 1; k <= h; k++)
            assert((at(0, -1) >= k || at(0, 1) >= k) && (at(-1, 0) >= k || at(1, 0) >= k));
          assert(std::abs(h - at(1, 0)) < 2 && std::abs(h - at(0, 1)) < 2);
          if (world::isRamp(x, y)) {
            int b = x - ((x % 3) + 3) % 3;
            assert(world::isRamp(b, y) && world::isRamp(b + 1, y) && world::isRamp(b + 2, y));
            ramps++;
          }
          peaks += h == 3;
          if (world::isWater(x, y)) {
            for (int d = 0; h && d < 9; d++)
              assert(at(d % 3 - 1, d / 3 - 1) == h);
            tarns += h > 0;
          } else {
            land[h >= 2]++;
            woods[h >= 2] += world::propAt(x, y, prop) && prop.solid > 0 && prop.solid < 4;
          }
          bool cliff = h > at(0, 1);
          run = cliff ? run + 1 : 0;
          cliffs += cliff;
          straight += run == 8 ? 8 : run > 8;
        }
  }
  noise_reseed(salt);
  float low = float(woods[0]) / land[0], high = float(woods[1]) / land[1];
  assert(peaks > 0 && ramps > 0 && tarns > 0 && straight * 5 < cliffs && high < low * 0.8f);
  std::cout << "[TEST] Hills rise in clean organic terraces: " << peaks << " hilltop tiles, "
            << ramps << " stair tiles, " << straight << "/" << cliffs
            << " cliff tiles in straight runs, " << tarns << " tarn tiles, woods "
            << int(low * 100) << "% low and " << int(high * 100) << "% high: PASS" << std::endl;
}

static void testClassKits() {
  {
    Game game;
    game.level = std::make_unique<Level>(game);
    Player &p = *game.level->player;
    for (const Hero *h : HEROES) {
      int weapons = 0, skills = 0, kin = 0;
      for (const Recipe &r : RECIPES)
        weapons += std::strcmp(r.owner, h->name) == 0;
      for (auto &u : p.upgrades) {
        if (!u->owner || std::strcmp(u->owner, h->name) != 0)
          continue;
        skills += u->skill;
        kin += !u->skill && !u->weapon;
      }
      if (weapons < 2 || skills < 3 || kin < 1)
        std::cout << "[TEST] Thin class kit: " << h->name << std::endl;
      assert(weapons >= 2 && skills >= 3 && kin >= 1);
    }
    game.level.reset();
  }
  auto recipe = [](const char *name) -> const Recipe & {
    for (const Recipe &r : RECIPES)
      if (std::strcmp(r.name, name) == 0)
        return r;
    assert(false);
    return RECIPES.front();
  };
  {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    p.hp = p.maxHp * 0.5f, p.regen = 0;
    float hp = p.hp;
    ability::banner(p, 90, 8);
    for (int i = 0; i < 120; i++)
      level.tick();
    assert(p.hp > hp);
    game.level.reset();
  }
  {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    RecipeShot shot(recipe("Hawk Talon"), p.x, p.bodyY(), 1, 0, 1, 170);
    shot.level = &level;
    double far = 0;
    for (int i = 0; i < 600 && !shot.removed; i++) {
      shot.tick();
      far = std::max(far, shot.x - p.x);
    }
    assert(shot.removed && shot.back && far > 120);
    game.level.reset();
  }
  {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    const Recipe &trap = recipe("Bear Trap");
    RecipeShot *shot = level.add<RecipeShot>(trap, p.x + 120, p.y, 0, 0, 30, trap.reach);
    for (int i = 0; i < 120; i++)
      level.tick();
    assert(!shot->removed);
    auto made = Enemy::create(EnemyKind::Knight, 200, 1);
    Enemy *e = made.get();
    e->x = p.x + 120, e->y = p.y, e->aware = false, e->plan = nullptr;
    level.add(std::move(made));
    float hp = e->hp;
    for (int i = 0; i < 10 && !shot->removed; i++)
      level.tick();
    assert(shot->removed && e->hp < hp);
    game.level.reset();
  }
  {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    p.luck = -1;
    p.hp = p.maxHp * 0.3f, p.regen = 0;
    float hp = p.hp;
    Upgrade *wings = p.grant("Seraph Wings", 1);
    assert(wings);
    for (double dx : {30.0, -30.0, 0.0}) {
      auto made = Enemy::create(EnemyKind::Knight, 400, 0.01f);
      made->x = p.x + dx, made->y = p.y + (dx == 0 ? 36 : 0);
      made->aware = false, made->plan = nullptr;
      level.add(std::move(made));
    }
    for (int i = 0; i < 600 && wings->dealt <= 0; i++)
      level.tick();
    assert(wings->dealt > 0 && p.hp > hp);
    game.level.reset();
  }
  {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    auto has = [&](const char *name) {
      for (auto &u : p.upgrades)
        if (std::strcmp(u->name, name) == 0)
          return true;
      return false;
    };
    for (auto &u : p.upgrades) {
      auto *w = dynamic_cast<RecipeWeapon *>(u.get());
      if (!w)
        continue;
      assert(w->partner && w->evolvedName && has(w->partner));
    }
    const Recipe &cleave = recipe("Lionheart Cleave");
    RecipeWeapon w(cleave);
    for (int i = 0; i < Upgrade::MAX_LEVEL; i++)
      w.levelUp(p);
    int count = w.count();
    float dmg = w.damage(p);
    w.evolvedName = "Kingsbane Cleave";
    w.evolve();
    assert(w.count() == count + 1 && w.damage(p) > dmg * 1.9f &&
           std::strcmp(w.name, "Kingsbane Cleave") == 0);
    for (const char *cls : {"WARDEN", "TINKERER", "ORACLE", "CORSAIR"}) {
      int talents = 0;
      for (auto &u : p.upgrades)
        talents += dynamic_cast<ExpansionTalent *>(u.get()) && std::strcmp(u->owner, cls) == 0;
      assert(talents == 1);
    }
    game.level.reset();
  }
  {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    auto made = Enemy::create(EnemyKind::Knight, 400, 1);
    Enemy *e = made.get();
    e->x = p.x + 60, e->y = p.y, e->aware = false, e->plan = nullptr;
    level.add(std::move(made));
    level.tick();
    level.tick();
    ExpansionTalent heart(0), sight(2), plunder(3);
    for (ExpansionTalent *t : {&heart, &sight, &plunder})
      for (int i = 0; i < Upgrade::MAX_LEVEL; i++)
        t->levelUp(p);
    p.hp = p.maxHp * 0.5f;
    float hp = p.hp;
    heart.cast(p);
    assert(p.hp > hp && e->status.power(StatusKind::Chill) > 0.5f);
    sight.timer = 0;
    sight.tick(p);
    assert(e->status.power(StatusKind::Marked) > 0);
    int gems = int(level.count<Gem>());
    for (int i = 0; i < 40; i++)
      plunder.slay(p, *e);
    level.tick();
    assert(int(level.count<Gem>()) > gems);
    game.level.reset();
  }
  {
    Game game;
    game.codex.start = 0;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    Upgrade *echo = p.grant("Echoing Blink", 1);
    assert(echo && echo->signature && p.skill);
    p.cast();
    float hero = p.skillTime;
    assert(hero < p.skill->cooldown * p.haste - 0.01f);
    Skill *warp = nullptr;
    for (auto &u : p.upgrades)
      if (u->skill && std::strcmp(u->name, "TIME WARP") == 0)
        warp = static_cast<Skill *>(u.get());
    warp->levelUp(p);
    p.skillTime = 0;
    p.cast();
    assert(std::abs(p.skillTime - warp->cooldown * p.haste) < 0.001f);
    game.level.reset();
  }
  std::cout << "[TEST] Every class owns weapons, skills, a kin passive and evolutions; "
               "banners heal, talons return, traps spring, wings leech, talents work: PASS"
            << std::endl;
}

static void testGeneralSkills() {
  Game game;
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &p = *level.player;
  auto use = [&](const char *name) {
    for (auto &up : p.upgrades)
      if (up->skill && std::strcmp(up->name, name) == 0) {
        assert(!up->owner);
        static_cast<Skill *>(up.get())->use(p);
        return;
      }
    assert(false);
  };
  p.hp = p.maxHp - 40;
  p.afflict(StatusKind::Poison, 5, 3);
  use("BANDAGE");
  assert(p.hp > p.maxHp - 15 && p.status.power(StatusKind::Poison) == 0);
  double x0 = p.x;
  p.xAim = 1, p.yAim = 0;
  use("TUMBLE");
  assert(p.x > x0 + 30 && p.invuln > 0);
  Gem *g = level.add<Gem>(p.x + 300, p.y, 1);
  level.tick();
  assert(!g->magnet);
  use("LODESTAR");
  assert(g->magnet);
  game.level.reset();
  std::cout << "[TEST] General skills: bandage, tumble and lodestar for any class: PASS"
            << std::endl;
}

static void testRunVariety() {
  for (int w = 0; w < Spawner::WAVES; w++) {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    level.time = 400;
    level.tick();
    int before = int(level.count<Enemy>());
    level.spawner.wave(w, 20);
    level.tick();
    assert(int(level.count<Enemy>()) > before);
    game.level.reset();
  }
  for (int i = 0; i < OMEN_COUNT; i++) {
    const Omen &o = OMENS[i];
    assert(o.name && *o.name && o.hp > 0 && o.rate > 0 && o.horde > 0 &&
           o.waves > 0 && o.bands > 0 && o.mimics > 0);
  }
  {
    Game game;
    game.level = std::make_unique<Level>(game);
    game.level->omen = -1;
    assert(&game.level->portent() == &CALM);
    game.level.reset();
  }
  for (int n = 0; n < 12; n++) {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    p.god = true;
    auto made = Enemy::create(EnemyKind::Knight, 50, 1);
    Enemy *e = made.get();
    e->x = p.x + 40, e->y = p.y, e->aware = true;
    e->makeElite(1);
    e->learn();
    assert(e->castCount == 1);
    level.add(std::move(made));
    bool wound = false, cast = false;
    for (int i = 0; i < 1500 && !cast; i++) {
      e->x = p.x + 40, e->y = p.y;
      if (e->wind > 0)
        wound = true;
      else if (wound)
        cast = true;
      level.tick();
    }
    assert(cast);
    game.level.reset();
  }
  for (const BossDef &d : BOSS_DEFS) {
    EnemyKind k = d.kind;
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    p.hp = p.maxHp = 1e6f;
    for (auto &up : p.upgrades)
      up->level = 0;
    Enemy *e = level.spawner.spawn(k, p.x + 60, p.y, true, k != EnemyKind::Cyclops);
    assert(e && e->castCount >= 2 && e->def == &d && e->title == d.title);
    level.each<Enemy>([&](Enemy &o) {
      if (&o != e)
        o.remove();
    });
    float hp = p.hp;
    double px = p.x, py = p.y;
    int casts = 0;
    for (int i = 0; i < 4800 && (casts < 3 || p.hp >= hp); i++) {
      p.invuln = 0, p.x = px, p.y = py;
      e->x = px + 60, e->y = py;
      bool winding = e->wind > 0;
      level.tick();
      casts += winding && e->wind <= 0;
    }
    assert(casts >= 2 && p.hp < hp);
    int before = int(level.count<Enemy>());
    e->hp = e->maxHp * 0.6f;
    level.tick();
    assert(e->phase == 1 && e->castCount == d.phases[1].count);
    if (d.minions)
      assert(int(level.count<Enemy>()) > before);
    e->hp = e->maxHp * 0.3f;
    level.tick();
    assert(e->phase == 2);
    game.level.reset();
  }
  assert(bossFor(BIOME_SNOW)->kind == EnemyKind::Iceback &&
         bossFor(BIOME_ASHLANDS)->kind == EnemyKind::CinderGolem);
  for (int n = 0; n < 6; n++) {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    level.time = 300;
    Enemy *c = level.spawner.champion(p.x + 200, p.y);
    assert(c && c->title && std::strlen(c->title) > 4 && c->elite && c->castCount == 1);
    int traits = 0;
    for (int t = 0; t < int(Trait::Count); t++)
      traits += c->traits.has(Trait(t));
    assert(traits == 2 && !c->upgrades.empty() && c->upgrades[0]->level >= 1 &&
           c->upgrades[0]->hostile());
    assert(std::find(level.bossIds.begin(), level.bossIds.end(), c->id) !=
           level.bossIds.end());
    std::string name = c->name();
    const char *title = c->title;
    int chests = 0;
    c->hurt(1e9f, 0, 0, 0);
    level.tick();
    for (auto [id, o] : level.reg.view<Owned>().each())
      chests += dynamic_cast<Chest *>(o.self.get()) != nullptr;
    assert(chests >= 1 && name == title);
    game.level.reset();
  }
  std::cout << "[TEST] Waves, omens, skilled elites, boss rotations and champions: PASS"
            << std::endl;
}

static void testNewTraits() {
  auto setup = [](Game &game, Trait t, double dx) {
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    p.hp = p.maxHp = 1e6f, p.luck = -1;
    for (auto &up : p.upgrades)
      up->level = 0;
    auto made = Enemy::create(EnemyKind::Knight, 50, 1);
    Enemy *e = made.get();
    e->x = p.x + dx, e->y = p.y, e->aware = false, e->plan = nullptr;
    e->traits.add(t);
    level.add(std::move(made));
    level.tick();
    return e;
  };
  {
    Game game;
    Enemy *e = setup(game, Trait::Thorned, 30);
    Player &p = *game.level->player;
    float hp = p.hp;
    p.invuln = 0;
    e->hurt(50, 0, 0, 0);
    assert(p.hp < hp);
    game.level.reset();
  }
  {
    Game game;
    Enemy *e = setup(game, Trait::Regenerating, 200);
    e->hurt(e->maxHp * 0.5f, 0, 0, 0);
    float hp = e->hp;
    for (int i = 0; i < 300; i++)
      game.level->tick();
    assert(e->hp > hp);
    game.level.reset();
  }
  {
    Game game;
    Enemy *e = setup(game, Trait::Blinking, 200);
    double x = e->x, y = e->y;
    e->hurt(5, 0, 0, 0);
    assert(std::abs(e->x - x) + std::abs(e->y - y) > 20);
    game.level.reset();
  }
  {
    Game game;
    Enemy *e = setup(game, Trait::Phasing, 200);
    e->traits.cycle = 3.5f;
    assert(e->traits.veiled() && e->traits.incoming(*e, 100) < 20);
    e->traits.cycle = 1;
    assert(!e->traits.veiled() && e->traits.incoming(*e, 100) == 100);
    game.level.reset();
  }
  {
    Game game;
    setup(game, Trait::Brooding, 200);
    int before = int(game.level->count<Enemy>());
    for (int i = 0; i < 480; i++)
      game.level->tick();
    assert(int(game.level->count<Enemy>()) > before);
    game.level.reset();
  }
  {
    Game game;
    Enemy *e = setup(game, Trait::Gravitic, 100);
    Player &p = *game.level->player;
    double ex = e->x, ey = e->y, x0 = p.x;
    for (int i = 0; i < 60; i++) {
      e->x = ex, e->y = ey;
      game.level->tick();
    }
    assert(p.x > x0 + 5);
    game.level.reset();
  }
  std::cout << "[TEST] Thorned, regenerating, blinking, phasing, brooding and gravitic traits: PASS"
            << std::endl;
}

static void testNewFoes() {
  {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    auto made = Enemy::create(EnemyKind::Mimic, 1, 1);
    Enemy *m = made.get();
    m->x = p.x + 160, m->y = p.y, m->aware = false;
    level.add(std::move(made));
    for (int i = 0; i < 10; i++)
      level.tick();
    assert(m->buried && level.nearest(p.x, p.y, 400, 8).empty());
    p.god = true;
    for (int i = 0; i < 60 && m->buried; i++) {
      p.x = m->x - 30, p.y = m->y;
      level.tick();
    }
    assert(!m->buried && !level.nearest(p.x, p.y, 400, 8).empty());
    game.level.reset();
  }
  for (int k = int(EnemyKind::Outlaw); k <= int(EnemyKind::Deserter); k++) {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    p.hp = p.maxHp = 1e6f;
    for (auto &up : p.upgrades)
      up->level = 0;
    auto made = Enemy::create(EnemyKind(k), 50, 1);
    auto *r = static_cast<Renegade *>(made.get());
    r->x = p.x + 70, r->y = p.y, r->aware = true;
    level.add(std::move(made));
    level.tick();
    assert(r->upgrades.size() == 1 && r->upgrades[0]->level > 0 &&
           r->upgrades[0]->hostile());
    float hp = p.hp;
    bool wound = false;
    double px = p.x, py = p.y;
    for (int i = 0; i < 1500 && !(wound && p.hp < hp); i++) {
      p.invuln = 0, p.x = px, p.y = py;
      wound |= r->wind > 0;
      level.tick();
    }
    assert(wound && p.hp < hp);
    game.level.reset();
  }
  std::cout << "[TEST] Mimics lurk unseen until near; renegades carry kits and cast: PASS"
            << std::endl;
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
#ifdef _WIN32
  _set_error_mode(_OUT_TO_STDERR);
  _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
  std::cout << "[TEST] Running memory leak & stability tests..." << std::endl;
  testPoolAllocAndFree();
  std::cout << "[TEST] Pool alloc/free & edge cases: PASS" << std::endl;
  testPoolConcurrency();
  std::cout << "[TEST] Pool multithreaded concurrency: PASS" << std::endl;
  testStatuses();
  testStatusDamage();
  testExpansionEnemies();
  testExpansionContent();
  testArsenal();
  testBiomeTransitions();
  testBiomeEncounters();
  testBalance();
  testExperienceCurve();
  testAutomaticLevelUp();
  testCombatReadability();
  testGuiChoices();
  testEnemyDeathEffects();
  testSidedAbilities();
  testSidedEffects();
  testFoeKits();
  testArtsBothSides();
  testGeneralSkills();
  testRunVariety();
  testNewTraits();
  testClassKits();
  testSpawnShore();
  testHills();
  testNewFoes();
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
