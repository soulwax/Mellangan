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

void testClassKits() {
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
    auto has = [&](const char *name) { return p.upgrade(name) != nullptr; };
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

void testGeneralSkills() {
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
