#pragma once

#include <cassert>
#include <cmath>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/ExpansionEnemy.hpp"
#include "level/Level.hpp"

inline void testExpansionEnemies() {
  Game game;
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  for (int variant = 0; variant < 10; variant++) {
    auto owned = std::make_unique<ExpansionEnemy>(
        Enemy::typeOf(EnemyKind::Rat), 1, 1, variant);
    ExpansionEnemy *e = owned.get();
    level.add(std::move(owned));
    e->x = p.x + 16;
    e->y = p.y;
    e->timer = e->pulse = 0;
    p.status.clear();
    e->tick();
    if (variant == 0 || variant == 1 || variant == 2 || variant == 3 ||
        variant == 5 || variant == 6) {
      assert(e->state == 1);
      e->timer = 0;
      e->tick();
      assert(e->state == (variant == 1 || variant == 6 ? 3 : 2));
      if (variant == 1)
        assert(p.status.power(StatusKind::Marked) > 0);
      if (variant == 0)
        assert(std::abs(e->x - p.x) <= 8);
      if (variant == 2 || variant == 3 || variant == 5)
        assert(e->xa * e->xa + e->ya * e->ya > 10000);
    }
    if (variant == 4 || variant == 5)
      assert(!e->afflict(StatusKind::Chill, 1, 0.3f));
    if (variant == 6 || variant == 7)
      assert(!e->afflict(StatusKind::Poison, 1, 3));
    if (variant >= 8)
      assert(!e->afflict(StatusKind::Burn, 1, 3));
    assert(std::isfinite(e->x) && std::isfinite(e->y));
    e->remove();
  }
  for (int i = 0; i < 240; i++)
    level.tick();
  assert(!p.dead());
  game.level.reset();
}
