#include "entity/projectile/MeteorRock.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/effect/FirePool.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Explosion.hpp"
#include "particle/Mark.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/Chip.hpp"

namespace {
constexpr Color FIRE{1, 0.6f, 0.25f, 1}, GOLD{1, 0.75f, 0.35f, 1};
}

void MeteorRock::tick() {
  t = std::min(1.0f, t + DT / FALL);
  if (t >= 1)
    return impact();
  float k = drop();
  if (Spark::alive < 600 && rng.chance(0.8f))
    level->add<Spark>(x + DX * k + rng.range(-3, 3),
                      y + DY * k + rng.range(-3, 3), DX * 0.15f,
                      DY * 0.15f, FIRE, rng.range(0.2f, 0.4f),
                      rng.range(0.4f, 0.7f), false);
}

void MeteorRock::impact() {
  level->enemiesNear(x, y, radius + 24, [&](Enemy &e) {
    double xd = e.x - x, yd = e.y - y;
    float r = radius + e.radius;
    if (xd * xd + yd * yd < r * r)
      e.hurt(damage, xd, yd, 120, source);
  });
  level->add<Explosion>(x, y, 0);
  level->add<Shockwave>(x, y, 6, radius * 1.5f, 0.4f, FIRE.alpha(0.9f));
  level->add<Mark>(x, y, 3.5f, atlas::Glow, Color{0.07f, 0.04f, 0.03f, 0.6f},
                   radius * 2.4f / atlas::Glow.w);
  level->add<Mark>(x, y, 3.5f, atlas::Ring, Color{0.22f, 0.13f, 0.08f, 0.55f},
                   radius * 1.9f / atlas::Ring.w);
  for (int i = 0; i < 9 && Debris::room(); i++) {
    float dir = rng.range(0, TAU), v = rng.range(1.2f, 2.4f);
    Chip *c = level->add<Chip>(x, y, 2, i % 3 ? atlas::pt::Pebble
                                              : atlas::pt::Rock,
                               col::white, 1);
    c->xa = std::cos(dir) * v, c->ya = std::sin(dir) * v * 0.7;
    c->za = rng.range(1.2f, 2.2f);
  }
  Spark::burst(*level, x, y, FIRE, 10, 120, true);
  level->add<FirePool>(x, y, radius, burn, 2)->source = source;
  Sound::explosion.play(0.5f);
  level->game.onExplosion();
  remove();
}

void MeteorRock::render(Screen &s) {
  float k = drop(), ring = radius * (1 + 0.6f * k);
  s.ring(xp, yp, ring, GOLD.alpha(0.7f * (1 - k)));
  float shadow = 0.4f + (1 - k) * 1.1f;
  s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, shadow * 1.4f, shadow);
  double bx = xp + DX * k, by = yp + DY * k;
  float tail = std::min(0.22f, 1 - k);
  s.line(bx, by, bx + DX * tail, by + DY * tail, 4, FIRE.alpha(0.35f));
  s.line(bx, by, bx + DX * tail * 0.6f, by + DY * tail * 0.6f, 2,
         Color{1, 0.92f, 0.6f, 0.6f});
  for (int i = 3; i >= 1; i--) {
    float back = i * 0.06f;
    s.color(FIRE.alpha(0.45f - i * 0.1f));
    s.center(atlas::Glow, bx + DX * back, by + DY * back,
             (14 - i * 3) / float(atlas::Glow.w));
  }
  s.color(col::white);
  s.particle(atlas::pt::MeteorCore, bx, by, col::white, 2, t * 14);
}

void MeteorRock::renderLight(Screen &s) {
  float k = drop();
  s.glow(xp + DX * k, yp + DY * k, 70, FIRE * 0.35f);
  s.glow(xp, yp, radius * 2.5f, GOLD * (0.12f * (1 - k)));
}
