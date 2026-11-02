#include "entity/projectile/Orb.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

void Orb::tick() {
  if ((life -= DT) <= 0)
    return remove();
  x += xa * DT;
  y += ya * DT;
  if (bounces > 0)
    ricochet();
  level->friends(x, y + 7, 20, [&](Mob &m) {
    double xd = m.x - x, yd = m.bodyY() - y;
    if (removed || xd * xd + yd * yd >= 7 * 7)
      return;
    if (Player *p = m.asPlayer())
      p->felledBy = by;
    m.strike(damage, xa, ya, 0);
    if (chill > 0)
      m.afflict(StatusKind::Chill, 0.5f, chill);
    Spark::burst(*level, x, y, tone, 6, 50, false);
    remove();
  });
}

void Orb::render(Screen &s) {
  s.line(xp - 5, yp - 5, xp - 2, yp - 5, 1, col::red);
  s.line(xp - 5, yp - 5, xp - 5, yp - 2, 1, col::red);
  s.line(xp + 5, yp + 5, xp + 2, yp + 5, 1, col::red);
  s.line(xp + 5, yp + 5, xp + 5, yp + 2, 1, col::red);
  s.color(tone);
  s.center(atlas::Bolt, xp, yp, size, life * 4);
  s.color(col::white);
  s.center(atlas::Dot, xp, yp, 0.4f);
}

void Orb::renderLight(Screen &s) {
  s.glow(xp, yp, 44, tone * 0.33f);
}

void Orb::ricochet() {
  double cx = x, cy = y + 7;
  level->collide(cx, cy, 2);
  double nx = cx - x, ny = cy - y - 7, n = std::sqrt(nx * nx + ny * ny);
  if (n < 1e-6)
    return;
  nx /= n, ny /= n;
  double dot = xa * nx + ya * ny;
  if (dot < 0)
    xa -= 2 * dot * nx, ya -= 2 * dot * ny;
  x = cx, y = cy - 7, bounces--;
  Spark::burst(*level, x, y, tone, 4, 40, false);
}
