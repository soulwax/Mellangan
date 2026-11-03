#undef NDEBUG
#include <cassert>
#ifdef _WIN32
#include <stdlib.h>
#endif

#include "test/Tests.hpp"

#include <stdlib.h>
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

void testCombatReadability() {
  std::vector<std::unique_ptr<Spark>> sparks;
  int shown = 0;
  for (int i = 0; i < 128; i++) {
    sparks.push_back(std::make_unique<Spark>(0.0, 0.0, 0.0, 0.0, col::white, 1.0f, 1.0f, false));
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

void testGuiChoices() {
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

void testEnemyDeathEffects() {
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

void testSidedAbilities() {
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
  entt::entity shot = hostile->id;
  for (int i = 0; i < 30 && level.reg.valid(shot); i++)
    level.tick();
  assert(p.hp < hp && e->hp == foeHp);

  p.invuln = 0, hp = p.hp;
  Arrow *friendly = level.add<Arrow>(p.x + 6, p.bodyY(), 300, 0, 10, 1);
  shot = friendly->id;
  for (int i = 0; i < 30 && level.reg.valid(shot); i++)
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

void testSidedEffects() {
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
  entt::entity shot = level.add<Sweep>(*e, 30, 10)->id;
  for (int i = 0; i < 40 && level.reg.valid(shot); i++)
    level.tick();
  assert(p.hp < hp);

  p.invuln = 0, hp = p.hp;
  Bomb *bomb = level.add<Bomb>(e->x, e->bodyY(), p.x, p.y, 10, 20, 1, 0.5f);
  bomb->hostile = true;
  shot = bomb->id;
  for (int i = 0; i < 60 && level.reg.valid(shot); i++)
    level.tick();
  assert(p.hp < hp);

  p.invuln = 0, hp = p.hp;
  shot = level.add<Slash>(*e, -1, 10, 1.2f)->id;
  for (int i = 0; i < 20 && level.reg.valid(shot); i++)
    level.tick();
  assert(p.hp < hp);

  p.invuln = 0, hp = p.hp;
  float foeHp = e->hp;
  level.add<Strike>(p.x, p.y, 10, 24)->hostile = true;
  level.tick();
  level.tick();
  assert(p.hp < hp && e->hp == foeHp);

  entt::entity orphan = level.add<Sweep>(*e, 30, 10)->id;
  e->hurt(1e6f, 0, 0, 0);
  for (int i = 0; i < 3; i++)
    level.tick();
  assert(!level.reg.valid(orphan));
  game.level.reset();
  std::cout << "[TEST] Sided sweeps, slashes, strikes, bombs and orphaned effects: PASS" << std::endl;
}

void testFoeKits() {
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

void testArtsBothSides() {
  const char *const ARTS[] = {"ARROW STORM", "SHIELD CHARGE", "MIASMA",
                              "CHAIN STORM", "DEATH GRIP", "SACRED LIGHT",
                              "LUNGE", "BRAMBLES", "MINEFIELD", "COMET",
                              "POWDER KEG", "WAR CRY", "WHIRLWIND",
                              "GROUND SLAM", "FIRE RING", "SNARE NET",
                              "HUNTER'S MARK", "FLASK BARRAGE", "TEMPEST",
                              "BLOOD PACT", "HALLOWED GROUND", "FLASH STEP",
                              "OVERGROWTH", "SMOKE BOMB", "BLOOD RITE",
                              "GRAVITY WELL", "RALLYING HORN", "SHATTER WAVE",
                              "SECOND SIGHT", "PRISMATIC PRISON", "CALL THE HAWK",
                              "LAST STAND", "TRANSMUTATION CIRCLE", "EYE OF THE STORM",
                              "RAISE DEAD", "HALO OF MERCY", "PERFECT PARRY",
                              "EARTHEN WALL", "OVERCLOCK", "FORESIGHT", "CANNONADE"};
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
