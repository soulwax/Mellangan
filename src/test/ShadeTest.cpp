#undef NDEBUG
#include <cassert>

#include <cmath>
#include <iostream>
#include <string>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Harvest.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/WildFoe.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"

namespace {

int spawned(Level &level, double x, double y) {
  int n = 0;
  for (int i = 0; i < 200; i++)
    if (Enemy *e = level.spawner.spawn(EnemyKind::Rat, x, y, false))
      n++, e->remove();
  level.tick();
  return n;
}

} // namespace

void testShades() {
  Game game;
  Campaign c;
  c.world = "SHADES01";
  game.level = std::make_unique<Level>(game, c.rules());
  Level &level = *game.level;
  c.enter(level);
  level.tick();
  Player &p = *level.player;
  p.god = true;

  double wx = c.xTown + 900, wy = c.yTown;
  int tx = int(std::floor(wx / 16)), ty = int(std::floor(wy / 16));
  int dark = spawned(level, wx + 40, wy);
  c.edits.place(tx, ty, Piece::Torch);
  int lit = spawned(level, wx + 40, wy);
  assert(lit < dark * 0.5f && dark > 150);

  p.x = wx + 200, p.y = wy + 120;
  level.night.dawn = false;
  auto made = Enemy::create(EnemyKind::Shade, 1, 1);
  auto *shade = dynamic_cast<WildFoe *>(made.get());
  shade->x = wx - 120, shade->y = wy - 40;
  level.add(std::move(made));
  p.felledBy = nullptr;
  for (int i = 0; i < 900 && c.edits.at(tx, ty)->piece == Piece::Torch; i++)
    level.tick();
  assert(c.edits.at(tx, ty)->piece == Piece::Snuffed);
  assert(Enemy::typeOf(EnemyKind::Shade).damage == 0 &&
         (!p.felledBy || std::string(p.felledBy) != "a shade"));

  p.materials[int(Mat::Wood)] = 0;
  p.x = tx * 16 + 8.0 + 3, p.y = ty * 16 + 16.0 + 3;
  Screen &view = game.screen;
  view.xOffset = int(p.x) - view.vw / 2, view.yOffset = int(p.y) - view.vh / 2;
  level.scatter(view);
  harvest::Target t;
  assert(harvest::target(level, c, t) && t.placed && t.tx == tx && t.ty == ty);
  assert(harvest::strike(level, c) && c.edits.at(tx, ty)->piece == Piece::Snuffed);
  p.materials[int(Mat::Wood)] = 1;
  assert(harvest::strike(level, c) && c.edits.at(tx, ty)->piece == Piece::Torch);
  assert(p.materials[int(Mat::Wood)] == 0);

  int cx = int(std::floor(c.xTown / 16)), cy = int(std::floor(c.yTown / 16));
  c.edits.place(cx + 4, cy + 3, Piece::Snuffed);
  c.barricade();
  assert(c.edits.at(cx + 4, cy + 3)->piece == Piece::Torch);

  int before = 0;
  level.each<Enemy>([&](Enemy &e) { before += e.is(EnemyKind::Shade); });
  p.x = wx, p.y = wy + 30;
  c.shadeTime = 0;
  c.tick(level);
  level.tick();
  int after = 0;
  level.each<Enemy>([&](Enemy &e) { after += e.is(EnemyKind::Shade); });
  assert(after == before + 1);
  level.night.dawn = true;
  level.tick();
  level.tick();
  after = 0;
  level.each<Enemy>([&](Enemy &e) { after += e.is(EnemyKind::Shade); });
  assert(after == 0);
  c.leave();
  std::cout << "[TEST] Shades: torches turn spawns away, a shade drifts to a torch and snuffs it "
               "without touching the hero, wood relights it, town torches return on entry, "
               "shades come to lit ground at night and fade at dawn: PASS"
            << std::endl;
}
