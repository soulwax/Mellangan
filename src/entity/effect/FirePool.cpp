#include "entity/effect/FirePool.hpp"

#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

void FirePool::tick() {
  if ((life -= DT) <= 0)
    return remove();
  if ((timer -= DT) <= 0) {
    timer = 0.4f;
    level->enemiesNear(x, y, radius + 24, [&](Enemy &e) {
      double xd = e.x - x, yd = e.y - y;
      float r = radius + e.radius;
      if (xd * xd + yd * yd < r * r) {
        e.afflict(StatusKind::Burn, 1.2f, damage * 0.4f, source);
        e.hurt(damage, 0, 0, 0, source);
      }
    });
  }
  if (rng.chance(0.6f) && Spark::alive < 500) {
    float dir = rng.range(0, TAU), r = rng.range(0, radius);
    level->add<Spark>(x + std::cos(dir) * r, y + std::sin(dir) * r,
                      rng.range(-6, 6), -rng.range(15, 35),
                      Color{1, rng.range(0.35f, 0.7f), 0.15f, 1},
                      rng.range(0.3f, 0.6f), rng.range(0.3f, 0.6f), false);
  }
}

void FirePool::render(Screen &s) {
  float fade = std::min(1.0f, life * 2) * std::min(1.0f, (maxLife - life) * 6);
  s.color({1, 0.45f, 0.12f, 0.35f * fade});
  s.center(atlas::Glow, xp, yp, radius * 2.4f / atlas::Glow.w);
  s.color({1, 0.6f, 0.2f, 0.25f * fade});
  s.center(atlas::Ring, xp, yp, radius * 2 / atlas::Ring.w, -life);
  s.color(col::white);
}

void FirePool::renderLight(Screen &s) {
  float flicker = 0.85f + 0.15f * std::sin(life * 23) * std::sin(life * 7);
  s.glow(xp, yp, radius * 4,
         Color{0.45f, 0.22f, 0.05f, 1} * (flicker * std::min(1.0f, life * 2)));
}
