#include "entity/effect/Smite.hpp"

#include "entity/enemy/Enemy.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "particle/Mark.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace {
constexpr Color HOLY{1, 0.85f, 0.4f, 1}, PALE{1, 0.97f, 0.82f, 1};
}

Smite::Smite(double x, double y, float damage, float radius) : damage(damage) {
  this->x = x, this->y = y, this->radius = radius;
}

void Smite::tick() {
  t += DT;
  if (!struck && t >= WINDUP)
    strike();
  if (t >= WINDUP + BEAM)
    remove();
}

void Smite::strike() {
  struck = true;
  level->foes((source && source->hostile()), x, y, radius + 24, [&](Mob &e) {
    double xd = e.x - x, yd = e.bodyY() - y;
    float r = radius + e.radius;
    if (xd * xd + yd * yd < r * r)
      e.strike(damage, e.x - x, e.y - y, 50, source);
  });
  level->add<Shockwave>(x, y, 4, radius * 1.7f, 0.35f, HOLY.alpha(0.9f));
  level->add<Mark>(x, y, 1.1f, atlas::Ring, HOLY.alpha(0.6f),
                   radius * 2 / atlas::Ring.w);
  level->add<Mark>(x, y, 0.8f, atlas::pt::HolyGlint, PALE, 1.5f);
  for (int i = 0; i < 10 && Spark::alive < 600; i++) {
    float dir = rng.range(0, TAU), d = rng.range(0, radius);
    level->add<Spark>(x + std::cos(dir) * d, y + std::sin(dir) * d * 0.5,
                      rng.range(-6, 6), -rng.range(30, 70), HOLY,
                      rng.range(0.4f, 0.8f), rng.range(0.4f, 0.7f), i < 2);
  }
}

void Smite::render(Screen &s) {
  if (!struck) {
    float k = t / WINDUP;
    s.ring(xp, yp, radius * (1.3f - 0.3f * k), HOLY.alpha(0.4f + 0.5f * k));
    s.particle(atlas::pt::Rune, xp, yp, HOLY.alpha(0.4f + 0.6f * k), 1.5f,
               k * 2);
    return;
  }
  float k = (t - WINDUP) / BEAM, w = 14 * (1 - k) + 2, a = 1 - k * k;
  for (int i = 0; i < 4; i++) {
    float top = yp - 60.0f * (i + 1), fade = 1 - i * 0.24f;
    s.rect(xp - w / 2 - 3, top, w + 6, 60, HOLY.alpha(0.3f * a * fade));
    s.rect(xp - w / 2, top, w, 60, PALE.alpha(0.8f * a * fade));
  }
  s.color(PALE.alpha(0.7f * a));
  s.center(atlas::Glow, xp, yp, (radius * 1.6f + 10) / atlas::Glow.w);
  s.color(col::white);
}

void Smite::renderLight(Screen &s) {
  float a = struck ? 1 - (t - WINDUP) / BEAM : t / WINDUP * 0.3f;
  s.glow(xp, yp - 20, 170, Color{0.45f, 0.36f, 0.12f, 1} * a);
}
