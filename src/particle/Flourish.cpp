#include "particle/Flourish.hpp"

#include "level/Level.hpp"
#include "particle/Drift.hpp"
#include "particle/Mark.hpp"
#include "particle/Shockwave.hpp"

namespace flourish {

void sigil(Level &level, double x, double y, float r, Color c, int spokes) {
  level.add<Shockwave>(x, y, r * 0.2f, r, 0.45f, c)->spokes = spokes;
  level.add<Mark>(x, y, 0.8f, atlas::pt::Rune, c.alpha(0.9f), 3, 0);
  level.add<Mark>(x, y, 0.6f, atlas::Glow, c.alpha(0.25f),
                  r * 1.6f / atlas::Glow.w);
}

void echo(Level &level, double x, double y, Sprite body, Color c) {
  level.add<Mark>(x, y - 8, 0.35f, body, c.alpha(0.6f), 1);
}

void streak(Level &level, double x0, double y0, double x1, double y1, int icon,
            Color c, int n) {
  for (int i = 0; i < n && Drift::room(); i++) {
    double k = (i + 0.5) / n;
    Drift *d = level.add<Drift>(x0 + (x1 - x0) * k, y0 + (y1 - y0) * k,
                                rng.range(-10, 10), rng.range(-20, -4), icon, c,
                                rng.range(0.4f, 0.7f));
    d->spin = rng.range(-4, 4), d->sway = 6, d->scale = 0.8f;
  }
}

void scatter(Level &level, double x, double y, int icon, Color c, int n,
             float speed, float gravity) {
  for (int i = 0; i < n && Drift::room(); i++) {
    float a = TAU * i / n + rng.range(-0.2f, 0.2f), v = speed * rng.range(0.6f, 1);
    Drift *d = level.add<Drift>(x, y, std::cos(a) * v, std::sin(a) * v, icon, c,
                                rng.range(0.5f, 0.9f));
    d->spin = rng.range(-5, 5), d->gravity = gravity, d->drag = 3;
  }
}

} // namespace flourish
