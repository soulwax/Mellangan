#include "entity/effect/Strike.hpp"

#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

Strike::Strike(double x, double y, float damage, float radius)
    : damage(damage) {
  this->x = x, this->y = y, this->radius = radius;
  for (float &j : jag)
    j = rng.range(-7, 7);
}

void Strike::tick() {
  if (!struck) {
    struck = true;
    level->enemiesNear(x, y, radius + 24, [&](Enemy &e) {
      double xd = e.x - x, yd = e.bodyY() - y;
      float r = radius + e.radius;
      if (xd * xd + yd * yd < r * r)
        e.hurt(damage, e.x - x, e.y - y, 50);
    });
    Spark::burst(*level, x, y, {0.7f, 0.85f, 1, 1}, 10, 90, true);
  }
  if ((life -= DT) <= 0)
    remove();
}

void Strike::render(Screen &s) {
  float a = life / 0.3f;
  double x0 = xp, y0 = yp - 150;
  for (size_t i = 0; i < jag.size(); i++) {
    float k = float(i + 1) / float(jag.size());
    double x1 = xp, y1 = yp;
    if (i + 1 < jag.size())
      x1 = xp + jag[i], y1 = yp - 150 * (1 - k);
    s.line(x0, y0, x1, y1, 3, {0.55f, 0.75f, 1, a * 0.7f});
    s.line(x0, y0, x1, y1, 1, col::white.alpha(a));
    x0 = x1, y0 = y1;
  }
  s.color({0.7f, 0.85f, 1, a * 0.6f});
  s.center(atlas::Ring, xp, yp, radius * 2 / atlas::Ring.w * (1.3f - 0.3f * a));
  s.color(col::white);
}

void Strike::renderLight(Screen &s) {
  s.glow(xp, yp, 160, Color{0.35f, 0.45f, 0.7f, 1} * (life / 0.3f));
}
