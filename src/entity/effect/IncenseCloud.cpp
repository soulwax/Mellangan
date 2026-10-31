#include "entity/effect/IncenseCloud.hpp"

#include <cmath>

#include "entity/enemy/Enemy.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "particle/Drift.hpp"
#include "particle/Spark.hpp"

IncenseCloud::IncenseCloud(double x, double y, double vx, double vy, float radius,
                           float damage, float duration)
    : damage(damage), life(duration), maxLife(duration) {
  this->x = x, this->y = y, this->radius = radius;
  this->xa = vx, this->ya = vy;
}

void IncenseCloud::tick() {
  if ((life -= DT) <= 0)
    return remove();
  x += xa * DT;
  y += ya * DT;
  xa *= 0.94;
  ya *= 0.94;

  if ((timer -= DT) <= 0) {
    timer = 0.35f;
    level->foes((source && source->hostile()), x, y, radius + 20, [&](Mob &e) {
      double xd = e.x - x, yd = e.y - y;
      float r = radius + e.radius;
      if (xd * xd + yd * yd < r * r)
        e.strike(damage, 0, 0, 0, source);
    });
  }

  if (rng.chance(0.13f) && Drift::room()) {
    float dir = rng.range(0, TAU), r = std::sqrt(rng.range(0.0f, 1.0f)) * radius;
    Drift *d = level->add<Drift>(x + std::cos(dir) * r, y + std::sin(dir) * r,
                                 rng.range(-4, 4), -rng.range(6, 14),
                                 atlas::pt::Puff, Color{1, 0.92f, 0.7f, 0.4f},
                                 rng.range(0.8f, 1.3f));
    d->grow = 1.1f, d->scale = 0.7f, d->sway = 6, d->spin = rng.range(-1, 1);
  }
  if (rng.chance(0.3f) && Spark::alive < 500) {
    float dir = rng.range(0, TAU), r = std::sqrt(rng.range(0.0f, 1.0f)) * radius;
    level->add<Spark>(x + std::cos(dir) * r, y + std::sin(dir) * r,
                      rng.range(-4, 4), -rng.range(10, 24),
                      Color{1.0f, rng.range(0.7f, 0.9f), 0.3f, 1.0f},
                      rng.range(0.4f, 0.7f), rng.range(0.3f, 0.5f), false);
  }
}

void IncenseCloud::render(Screen &s) {
  float fade = std::min(1.0f, life * 1.8f) * std::min(1.0f, (maxLife - life) * 5.0f);
  s.color(Color{0.95f, 0.8f, 0.35f, 0.08f * fade});
  s.center(atlas::Glow, xp, yp, radius * 2.2f / atlas::Glow.w);
  s.color(col::white);
}

void IncenseCloud::renderLight(Screen &s) {
  float fade = std::min(1.0f, life * 1.5f);
  s.glow(xp, yp, radius * 3.0f, Color{0.32f, 0.24f, 0.08f, 1} * fade);
}

