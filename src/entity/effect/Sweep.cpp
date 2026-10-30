#include "entity/effect/Sweep.hpp"

#include <algorithm>

#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "particle/Drift.hpp"
#include "particle/Spark.hpp"

Sweep::Sweep(const Player &p, float reach, float damage, SweepStyle style)
    : player(p), reach(reach), damage(damage), angle(rng.range(0, TAU)),
      start(angle), style(style) {
  x = p.x, y = p.bodyY();
}

Color Sweep::tone() const {
  if (style == SweepStyle::Steel)
    return {1, 0.78f, 0.9f, 1};
  if (style == SweepStyle::Rally)
    return {1, 0.86f, 0.5f, 1};
  if (source && source->evolved)
    return {1, 0.9f, 0.6f, 1};
  return {1, 0.36f, 0.42f, 1};
}

Sprite Sweep::blade() const {
  return style == SweepStyle::Reap ? atlas::town(atlas::tt::Scythe)
                                   : atlas::tile(atlas::t::Sword);
}

void Sweep::trail() {
  Color c = tone();
  if (style == SweepStyle::Reap) {
    if (Drift::room() && rng.chance(0.5f)) {
      Drift *d = level->add<Drift>(x, y, rng.range(-8, 8), rng.range(-14, -4),
                                   atlas::pt::Puff,
                                   Color{0.22f, 0.08f, 0.14f, 0.55f},
                                   rng.range(0.4f, 0.7f));
      d->grow = 1.4f, d->scale = 0.6f;
    }
    if (rng.chance(0.35f) && Spark::alive < 600)
      level->add<Spark>(x, y, rng.range(-20, 20), rng.range(-30, -5), c, 0.4f,
                        0.4f, false);
    return;
  }
  if (style == SweepStyle::Steel) {
    if (Drift::room() && rng.chance(0.3f)) {
      Drift *d = level->add<Drift>(x, y, rng.range(-15, 15), rng.range(-20, 0),
                                   atlas::pt::Petal, col::white,
                                   rng.range(0.5f, 0.8f));
      d->spin = rng.range(-5, 5), d->sway = 12, d->gravity = 25;
      d->scale = 0.8f;
    }
    return;
  }
  if (rng.chance(0.6f) && Spark::alive < 600)
    level->add<Spark>(x, y, rng.range(-25, 25), rng.range(-40, -10), c, 0.4f,
                      0.5f, rng.chance(0.2f));
}

void Sweep::tick() {
  t += DT / 0.45f;
  angle = start + TAU * std::min(t, 1.0f);
  x = player.x + std::cos(angle) * reach;
  y = player.bodyY() + std::sin(angle) * reach;
  level->enemiesNear(x, y, 40, [&](Enemy &e) {
    double xd = e.x - x, yd = e.bodyY() - y;
    float r = 13 + e.radius;
    if (xd * xd + yd * yd >= r * r ||
        std::find(hits.begin(), hits.end(), e.id) != hits.end())
      return;
    hits.push_back(e.id);
    e.hurt(damage, e.x - player.x, e.y - player.y, 120, source);
  });
  trail();
  if (t >= 1)
    remove();
}

void Sweep::render(Screen &s) {
  int xc = int(std::floor(player.x)), yc = int(std::floor(player.bodyY()));
  Color c = tone();
  Sprite b = blade();
  float tilt = style == SweepStyle::Reap ? PI * 0.75f : PI * 0.25f;
  for (int i = 4; i >= 1; i--) {
    float a = angle - i * 0.22f;
    s.color(c.alpha(0.13f * float(5 - i)));
    s.center(b, xc + std::cos(a) * reach, yc + std::sin(a) * reach, 1.2f,
             a + tilt);
  }
  s.color(col::white);
  s.center(b, xp, yp, 1.2f, angle + tilt);
}

void Sweep::renderLight(Screen &s) {
  Color c = tone();
  s.glow(xp, yp, 60, Color{c.r * 0.2f, c.g * 0.2f, c.b * 0.25f, 1});
}
