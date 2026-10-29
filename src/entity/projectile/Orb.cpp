#include "entity/projectile/Orb.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

void Orb::tick() {
  if ((life -= DT) <= 0)
    return remove();
  x += xa * DT;
  y += ya * DT;
  Player &p = *level->player;
  double xd = p.x - x, yd = p.bodyY() - y;
  if (!p.dead() && xd * xd + yd * yd < 7 * 7) {
    p.felledBy = by;
    p.hurt(damage);
    Spark::burst(*level, x, y, {0.75f, 0.35f, 1, 1}, 6, 50, false);
    remove();
  }
}

void Orb::render(Screen &s) {
  s.line(xp - 5, yp - 5, xp - 2, yp - 5, 1, col::red);
  s.line(xp - 5, yp - 5, xp - 5, yp - 2, 1, col::red);
  s.line(xp + 5, yp + 5, xp + 2, yp + 5, 1, col::red);
  s.line(xp + 5, yp + 5, xp + 5, yp + 2, 1, col::red);
  s.color({0.75f, 0.35f, 1, 1});
  s.center(atlas::Bolt, xp, yp, 1.2f, life * 4);
  s.color(col::white);
  s.center(atlas::Dot, xp, yp, 0.4f);
}

void Orb::renderLight(Screen &s) {
  s.glow(xp, yp, 44, {0.25f, 0.08f, 0.35f, 1});
}
