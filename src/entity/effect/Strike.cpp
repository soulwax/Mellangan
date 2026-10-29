#include "entity/effect/Strike.hpp"

#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Mark.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

Strike::Strike(double x, double y, float damage, float radius)
    : damage(damage) {
  this->x = x, this->y = y, this->radius = radius;
  for (float &j : jag)
    j = rng.range(-7, 7);
  float side = rng.chance(0.5f) ? 1.0f : -1.0f;
  for (int i = 0; i < 3; i++)
    fork[i] = side * (8 + i * 9 + rng.range(-3, 3));
}

void Strike::tick() {
  if (!struck) {
    struck = true;
    level->enemiesNear(x, y, radius + 24, [&](Enemy &e) {
      double xd = e.x - x, yd = e.bodyY() - y;
      float r = radius + e.radius;
      if (xd * xd + yd * yd < r * r)
        e.hurt(damage, e.x - x, e.y - y, 50, source);
    });
    Spark::burst(*level, x, y, {0.7f, 0.85f, 1, 1}, 18, 90, true);
    level->add<Shockwave>(x, y, 4, radius * 1.8f, 0.3f,
                          Color{0.75f, 0.88f, 1, 0.8f}, false);
    level->add<Mark>(x, y, 1.2f, atlas::Glow, Color{0.04f, 0.04f, 0.08f, 0.5f},
                     radius * 2.2f / atlas::Glow.w);
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
    if (i == 2) {
      double fx = x1, fy = y1;
      for (size_t f = 0; f < fork.size(); f++) {
        double gx = x1 + fork[f], gy = y1 + 14.0 * (f + 1);
        s.line(fx, fy, gx, gy, 2, {0.55f, 0.75f, 1, a * 0.5f});
        s.line(fx, fy, gx, gy, 1, col::white.alpha(a * 0.7f));
        fx = gx, fy = gy;
      }
    }
    x0 = x1, y0 = y1;
  }
}

void Strike::renderLight(Screen &s) {
  s.glow(xp, yp, 160, Color{0.35f, 0.45f, 0.7f, 1} * (life / 0.3f));
}
