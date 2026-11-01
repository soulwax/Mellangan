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

void testRunVariety() {
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

void testNewTraits() {
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

void testNewFoes() {
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
