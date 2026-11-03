#undef NDEBUG
#include <cassert>

#include "test/Tests.hpp"

#include <iostream>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"

void testBlend() {
  SpriteObject o;
  o.x = 10, o.y = 20, o.z = 2;
  o.blend(0.5f);
  assert(o.x == 10 && o.y == 20);
  o.remember();
  o.x = 18, o.y = 24, o.z = 6;
  o.blend(0.5f);
  assert(o.x == 14 && o.y == 22 && o.z == 4);
  o.unblend();
  assert(o.x == 18 && o.y == 24 && o.z == 6);
  o.unblend();
  assert(o.x == 18);
  o.remember();
  o.x += 200;
  o.blend(0.5f);
  assert(o.x == 218);
  o.unblend();
  assert(o.x == 218);

  Game game;
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &p = *level.player;
  Screen &s = game.screen;
  auto foe = Enemy::create(EnemyKind::Rat, 1, 1);
  Enemy *e = foe.get();
  e->x = p.x + 60, e->y = p.y;
  level.add(std::move(foe));
  level.tick();
  level.tick();
  assert(!std::isnan(e->xLast));
  s.xOffset = int(p.x) - s.vw / 2, s.yOffset = int(p.y) - s.vh / 2;
  level.remember();
  double x = p.x + 6, y = p.y - 3, ex = e->x + 8, eLast = e->xLast;
  p.x = x, p.y = y, e->x = ex;
  level.blend(0.25f, s);
  assert(std::abs(p.x - (x - 4.5)) < 1e-9 && std::abs(p.y - (y + 2.25)) < 1e-9);
  assert(std::abs(e->x - (eLast + (ex - eLast) * 0.25)) < 1e-9);
  level.unblend();
  assert(p.x == x && p.y == y && e->x == ex);
  level.blend(1, s);
  assert(p.x == x && p.y == y);
  game.level.reset();
  std::cout << "[TEST] Render blending eases between ticks, snaps teleports and restores exactly: PASS" << std::endl;
}
