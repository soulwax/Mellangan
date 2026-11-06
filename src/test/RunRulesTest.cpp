#undef NDEBUG
#include <cassert>

#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "entity/enemy/SupplyBarrel.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/pickup/Magnet.hpp"

void testRunRules() {
  {
    Game game;
    game.level = std::make_unique<Level>(game);
    Night &n = game.level->night;
    assert(n.dawnAt() == 900);
    n.lit = 2;
    assert(n.dawnAt() == 780);
    game.level.reset();
  }
  {
    RunRules rules;
    rules.nightLength = 30, rules.dayLength = 10, rules.endsAtDawn = false;
    rules.spawnRate = 0, rules.waves = false, rules.bosses = false;
    Game game;
    game.level = std::make_unique<Level>(game, rules);
    Level &level = *game.level;
    Player &p = *level.player;
    p.hp = p.maxHp = 1e9f;
    for (auto &up : p.upgrades)
      up->level = 0;
    bool sawDay = false;
    for (int i = 0; i < 95 * 60; i++) {
      level.tick();
      if (level.night.dawn)
        sawDay = true, assert(level.night.daylight() > 0);
    }
    assert(sawDay && level.night.nights >= 3);
    assert(!p.god);
    int foes = 0;
    level.each<Enemy>([&](Enemy &e) { foes += !dynamic_cast<SupplyBarrel *>(&e); });
    assert(foes < 8);
    game.level.reset();
  }
  {
    RunRules rules;
    rules.omen = 3;
    Game game;
    game.level = std::make_unique<Level>(game, rules);
    assert(game.level->omen == 3);
    rules.omen = -1;
    game.level = std::make_unique<Level>(game, rules);
    assert(game.level->omen == -1);
    game.level.reset();
  }
  {
    RunRules rules;
    Game game;
    game.level = std::make_unique<Level>(game, rules);
    Player &p = *game.level->player;
    rules.safe.push_back({p.x, p.y, 200});
    rules.threat = [](double, double) { return 0.0f; };
    game.level = std::make_unique<Level>(game, rules);
    Level &level = *game.level;
    Player &q = *level.player;
    double qx = q.x, qy = q.y;
    assert(!level.spawner.spawn(EnemyKind::Rat, q.x + 10, q.y));
    level.time = 600;
    Enemy *far = level.spawner.spawn(EnemyKind::Rat, q.x + 500, q.y);
    assert(far && !far->elite);
    float calm = far->maxHp;
    RunRules plain;
    plain.omen = level.omen;
    game.level = std::make_unique<Level>(game, plain);
    Enemy *base = game.level->spawner.spawn(EnemyKind::Rat, qx + 500, qy);
    assert(base && std::abs(base->maxHp - calm) < 1e-3f);
    game.level.reset();
  }
  std::cout << "[TEST] Run rules: default dawn, day and night cycle, quiet spawner, fixed omen, "
               "safe zones, threat by place: PASS"
            << std::endl;
}

namespace {
bool shrineOnly(const Structure *s, const void *) { return s->kind == StructureKind::Shrine; }
} // namespace

void testLodestones() {
  Rng kept = rng;
  {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    for (int i = 0; i < 20; i++) {
      auto b = std::make_unique<SupplyBarrel>();
      b->x = level.player->x + 200, b->y = level.player->y;
      SupplyBarrel *at = b.get();
      level.add(std::move(b));
      at->hurt(1e6f, 0, 0, 0);
    }
    level.tick();
    int stones = 0;
    for (auto [id, o] : level.reg.view<Owned>().each())
      stones += dynamic_cast<Magnet *>(o.self.get()) && !o.self->removed;
    assert(stones == 20);
    game.level.reset();
  }
  {
    RunRules rules;
    rules.spawnRate = 0, rules.waves = false, rules.bosses = false;
    Game game;
    game.level = std::make_unique<Level>(game, rules);
    Level &level = *game.level;
    Player &p = *level.player;
    Structure s;
    assert(world::nearestStructure(p.x, p.y, 8, s, shrineOnly));
    p.x = s.centerX(), p.y = s.centerY(), p.god = true;
    Gem *nearGem = level.add<Gem>(p.x + 150, p.y, 1.0f), *farGem = level.add<Gem>(p.x + 400, p.y, 1.0f);
    for (int i = 0; i < 900 && !level.night.offering; i++) {
      p.x = s.centerX(), p.y = s.centerY();
      level.tick();
    }
    assert(level.night.offering && nearGem->magnet && !farGem->magnet);
    game.level.reset();
  }
  rng = kept;
  std::cout << "[TEST] Lodestones: every supply barrel drops one, and a kindled shrine pulls the gems "
               "around it: PASS"
            << std::endl;
}
