#include "particle/DeathEffect.hpp"

#include "level/Level.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/FlameDebris.hpp"

DeathEffect::DeathEffect(double x, double y, float height, Sprite body,
                         float size, float facing, Color tint, DeathStyle style)
    : Particle(x, y, style == DeathStyle::Fallen ? 1.6f : 1.0f), style(style),
      size(size), facing(facing), height(height), tint(tint) {
  sprite = body;
  z = height;
  alive++;
}

void DeathEffect::tick() {
  Particle::tick();
  if (removed)
    return;
  float t = 1 - frac();
  if (style == DeathStyle::Spirit)
    z = height + t * 30;
  else if (style == DeathStyle::Fallen)
    z = height * std::max(0.0f, 1 - t * 5);
  if (style == DeathStyle::Flame && rng.chance(0.45f))
    FlameDebris::lick(*level, x + rng.range(-5, 5) * size, y,
                      rng.range(3, 13) * size, 1);
  if ((style == DeathStyle::Spirit || style == DeathStyle::Slime) &&
      rng.chance(0.4f) && Spark::alive < 500) {
    bool spirit = style == DeathStyle::Spirit;
    level->add<Spark>(x + rng.range(-6, 6) * size,
                      y - z - rng.range(2, 12) * size,
                      rng.range(-8, 8), spirit ? -rng.range(20, 40) : rng.range(0, 4),
                      tint, rng.range(0.35f, 0.7f), rng.range(0.3f, 0.6f), false);
  }
}

void DeathEffect::render(Screen &s) {
  float t = 1 - frac();
  float fade = style == DeathStyle::Fallen ? std::min(1.0f, frac() * 3) : frac();
  float sx = size * facing, sy = size, rotation = 0;
  if (style == DeathStyle::Slime) {
    sx *= 1 + t * 0.5f;
    sy *= std::max(0.08f, 1 - t);
  } else if (style == DeathStyle::Fallen) {
    float fall = std::min(1.0f, t * 5);
    rotation = facing * fall * 1.25f;
    sy *= 1 - 0.35f * fall;
  } else if (style == DeathStyle::Flame) {
    sy *= 1 - t * 0.7f;
  }
  s.color(col::white.alpha(fade));
  s.sprite(sprite, xp, yp, 8, 16, sx, sy, rotation);
  s.color(col::white);
}
