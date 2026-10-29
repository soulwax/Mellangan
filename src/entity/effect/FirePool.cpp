#include "entity/effect/FirePool.hpp"

#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Mark.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/FlameDebris.hpp"

namespace {
constexpr Color ACID{0.55f, 0.95f, 0.3f, 1};
}

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
  float dir = rng.range(0, TAU), r = std::sqrt(rng.range(0.0f, 1.0f)) * radius;
  double px = x + std::cos(dir) * r, py = y + std::sin(dir) * r * 0.8;
  if (style == PoolStyle::Acid) {
    if (rng.chance(0.3f))
      level->add<Mark>(px, py, rng.range(0.3f, 0.6f), atlas::pt::Bubble,
                       col::white, rng.chance(0.3f) ? 1.0f : 0.75f);
    if (rng.chance(0.4f) && Spark::alive < 500)
      level->add<Spark>(px, py, rng.range(-3, 3), -rng.range(6, 14),
                        ACID.alpha(0.7f), rng.range(0.4f, 0.8f),
                        rng.range(0.3f, 0.5f), false);
    return;
  }
  if (rng.chance(0.35f))
    FlameDebris::lick(*level, px, py, 1, 0.8f);
  if (Spark::alive < 500)
    level->add<Spark>(px, py, rng.range(-6, 6), -rng.range(15, 35),
                      Color{1, rng.range(0.35f, 0.7f), 0.15f, 1},
                      rng.range(0.3f, 0.6f), rng.range(0.3f, 0.6f), false);
}

void FirePool::render(Screen &s) {
  float fade = std::min(1.0f, life * 2) * std::min(1.0f, (maxLife - life) * 6);
  bool acid = style == PoolStyle::Acid;
  s.color(acid ? Color{0.8f, 1, 0.25f, 0.24f * fade}
               : Color{1, 0.45f, 0.12f, 0.2f * fade});
  s.center(atlas::Glow, xp, yp, radius * 2.4f / atlas::Glow.w);
  s.color(col::white);
  if (acid)
    s.ring(xp, yp, radius * 0.9f, Color{0.12f, 0.3f, 0.06f, 0.6f * fade});
}

void FirePool::renderLight(Screen &s) {
  float flicker = 0.85f + 0.15f * std::sin(life * 23) * std::sin(life * 7);
  bool acid = style == PoolStyle::Acid;
  Color c = acid ? Color{0.05f, 0.12f, 0.02f, 1} : Color{0.45f, 0.22f, 0.05f, 1};
  s.glow(xp, yp, radius * (acid ? 2.5f : 4), c * (flicker * std::min(1.0f, life * 2)));
}
