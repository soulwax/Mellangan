#include "entity/projectile/Slash.hpp"

#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "particle/Drift.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/Chip.hpp"

Slash::Slash(Player &p, float side, float damage, float scale, SlashStyle style)
    : Projectile(p.x, p.bodyY(), 0, 0, 0.22f, damage, 90, 9999), player(p),
      side(side), scale(scale), style(style) {}

Color Slash::tone() const {
  if (style == SlashStyle::Crusader)
    return {1, 0.86f, 0.5f, 1};
  if (style == SlashStyle::Flurry)
    return {1, 0.68f, 0.84f, 1};
  if (source && source->evolved)
    return {0.78f, 0.62f, 1, 1};
  return {0.86f, 0.93f, 1, 1};
}

void Slash::emit() {
  emitted = true;
  Color c = tone();
  for (int i = 0; i < 6; i++) {
    float t = rng.range(-1, 1);
    double px = x + side * (6 - 6 * t * t) * scale, py = y + t * 20 * scale;
    if (style == SlashStyle::Crusader) {
      if (Spark::alive < 600)
        level->add<Spark>(px, py, side * rng.range(40, 110), rng.range(-50, 10),
                          c, rng.range(0.3f, 0.5f), rng.range(0.4f, 0.7f),
                          i < 2);
      continue;
    }
    if (!Drift::room())
      return;
    bool petal = style == SlashStyle::Flurry;
    if (!petal && i > 3)
      return;
    Drift *d = level->add<Drift>(
        px, py, side * rng.range(10, 40), rng.range(-30, -5),
        petal ? atlas::pt::Petal
              : i == 0 ? atlas::pt::Crescent : atlas::pt::FrostGlint,
        petal ? col::white : c.alpha(0.9f), rng.range(0.5f, 0.9f));
    d->spin = rng.range(-4, 4);
    d->sway = petal ? 14 : 4;
    d->gravity = petal ? 30 : -10;
    d->scale = petal ? 0.9f : 0.6f;
  }
  if (style == SlashStyle::Crusader) {
    double gx = x, gy = player.y;
    for (int i = 0; i < 3 && Debris::room(); i++)
      level->add<Chip>(gx, gy, 2, atlas::pt::Pebble, col::white, 0.9);
    if (Drift::room()) {
      Drift *d = level->add<Drift>(gx, gy - 2, side * 12, -6, atlas::pt::Puff,
                                   Color{0.85f, 0.78f, 0.62f, 0.55f}, 0.6f);
      d->grow = 1.2f;
    }
  }
}

void Slash::tick() {
  if ((life -= DT) <= 0)
    return remove();
  x = player.x + side * 16 * scale;
  y = player.bodyY();
  if (!emitted)
    emit();
  collide();
}

bool Slash::touches(const Enemy &e) const {
  double xd = e.x - x, yd = e.bodyY() - y;
  float rx = 14 * scale + e.radius * 0.5f, ry = 24 * scale;
  return (xd * xd) / (rx * rx) + (yd * yd) / (ry * ry) <= 1;
}

void Slash::render(Screen &s) {
  float f = frac();
  s.color(tone().alpha(std::pow(f, 0.6f)));
  s.sprite(atlas::Slash, xp, yp, 12, 24, side * scale * (0.8f + 0.4f * (1 - f)),
           scale);
  s.color(col::white);
}

void Slash::renderLight(Screen &s) {
  Color c = tone();
  s.glow(xp, yp, 80 * scale, Color{c.r * 0.17f, c.g * 0.16f, c.b * 0.13f, 1} *
                                 frac());
}
