#undef NDEBUG
#include <cassert>

#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "entity/enemy/SupplyBarrel.hpp"
#include "level/Level.hpp"

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
    assert(!level.spawner.spawn(EnemyKind::Rat, q.x + 10, q.y));
    level.time = 600;
    Enemy *far = level.spawner.spawn(EnemyKind::Rat, q.x + 500, q.y);
    assert(far && !far->elite);
    float calm = far->maxHp;
    RunRules plain;
    plain.omen = level.omen;
    game.level = std::make_unique<Level>(game, plain);
    Enemy *base = game.level->spawner.spawn(EnemyKind::Rat, q.x + 500, q.y);
    assert(base && std::abs(base->maxHp - calm) < 1e-3f);
    game.level.reset();
  }
  std::cout << "[TEST] Run rules: default dawn, day and night cycle, quiet spawner, fixed omen, "
               "safe zones, threat by place: PASS"
            << std::endl;
}
