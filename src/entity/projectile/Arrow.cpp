#include "entity/projectile/Arrow.hpp"

#include "level/Level.hpp"
#include "particle/Drift.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace {
constexpr Color WIND{0.6f, 0.95f, 0.5f, 0.4f}, STEEL{1, 1, 1, 0.55f},
    FLETCH{0.95f, 0.88f, 0.75f, 0.35f};
}

void Arrow::tick() {
  if ((life -= DT) <= 0)
    return remove();
  x += xa * DT;
  y += ya * DT;
  collide();
  if (style == ArrowStyle::Piercing && hits.size() > pierced) {
    pierced = hits.size();
    level->add<Shockwave>(x, y, 2, 10, 0.18f, STEEL.alpha(0.9f), false);
    Spark::burst(*level, x, y, col::white, 4, 60, false);
  }
  int icon = style == ArrowStyle::Hunter ? atlas::pt::Leaf :
             style == ArrowStyle::Scatter ? atlas::pt::Feather : -1;
  if (icon >= 0 && rng.chance(0.05f) && Drift::room()) {
    Drift *d = level->add<Drift>(x, y, xa * 0.05, ya * 0.05, icon, col::white,
                                 rng.range(0.5f, 0.8f));
    d->spin = rng.range(-3, 3), d->sway = 10, d->gravity = 20;
    d->scale = 0.8f;
  }
}

void Arrow::render(Screen &s) {
  double len = std::max(1.0, std::sqrt(xa * xa + ya * ya));
  double bx = -xa / len, by = -ya / len;
  if (style == ArrowStyle::Piercing) {
    s.line(xp, yp, xp + bx * 24, yp + by * 24, 2, STEEL);
    s.line(xp - by * 3 + bx * 6, yp + bx * 3 + by * 6, xp - by * 3 + bx * 18,
           yp + bx * 3 + by * 18, 1, STEEL.alpha(0.3f));
    s.line(xp + by * 3 + bx * 6, yp - bx * 3 + by * 6, xp + by * 3 + bx * 18,
           yp - bx * 3 + by * 18, 1, STEEL.alpha(0.3f));
  } else {
    s.line(xp, yp, xp + bx * 12, yp + by * 12, 1,
           style == ArrowStyle::Hunter ? WIND : FLETCH);
  }
  s.center(atlas::town(atlas::tt::Arrow), xp, yp,
           style == ArrowStyle::Piercing ? 1.25f : 1,
           float(std::atan2(ya, xa)) + PI / 4);
}
