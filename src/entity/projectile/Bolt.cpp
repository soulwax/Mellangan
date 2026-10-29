#include "entity/projectile/Bolt.hpp"

#include "entity/enemy/Enemy.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "particle/Mark.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace {
struct Look {
  Color halo, core, light;
  int icon;
};
const Look LOOKS[] = {
    {{0.72f, 0.45f, 1, 1}, {0.95f, 0.85f, 1, 1}, {0.18f, 0.08f, 0.32f, 1},
     atlas::pt::ArcaneStar},
    {{0.2f, 0.14f, 0.3f, 1}, {0.94f, 0.96f, 1, 1}, {0.1f, 0.1f, 0.18f, 1},
     atlas::pt::Wisp},
    {{1, 0.8f, 0.35f, 1}, {1, 0.95f, 0.75f, 1}, {0.3f, 0.2f, 0.05f, 1}, -1},
};
} // namespace

Bolt::Bolt(double x, double y, double xa, double ya, float damage, int pierce,
           entt::entity target, BoltStyle style)
    : Projectile(x, y, xa, ya, 1.6f, damage, 45, pierce), target(target),
      style(style) {}

float Bolt::scale() const { return source && source->evolved ? 1.4f : 1.0f; }

void Bolt::tick() {
  age += DT;
  if ((life -= DT) <= 0)
    return remove();
  if (Enemy *e = level->get<Enemy>(target)) {
    double xd = e->x - x, yd = e->bodyY() - y;
    double dd = std::sqrt(xd * xd + yd * yd);
    if (dd > 1e-5) {
      double k = damp(6, DT);
      xa += (xd / dd * 230 - xa) * k;
      ya += (yd / dd * 230 - ya) * k;
    }
  }
  x += xa * DT;
  y += ya * DT;
  if (rng.chance(0.5f) && Spark::alive < 600) {
    float dir = rng.range(0, TAU);
    double drift = style == BoltStyle::Wraith ? 0.05 : 0.1;
    level->add<Spark>(x, y, xa * -drift + std::cos(dir) * 8,
                      ya * -drift + std::sin(dir) * 8 -
                          (style == BoltStyle::Wraith ? 10 : 0),
                      LOOKS[int(style)].halo, 0.25f, 0.35f, false);
  }
  collide();
}

void Bolt::render(Screen &s) {
  const Look &l = LOOKS[int(style)];
  float k = scale();
  bool smoke = style == BoltStyle::Wraith;
  s.color(l.halo.alpha(smoke ? 0.7f : 0.45f));
  s.center(atlas::Glow, xp, yp, (smoke ? 18 : 22) * k / atlas::Glow.w);
  s.color(col::white);
  if (style == BoltStyle::Arcane)
    return s.particle(l.icon, xp, yp, l.core, k, age * 8);
  if (style == BoltStyle::Wraith)
    return s.particle(l.icon, xp, yp, l.core, k,
                      float(std::atan2(-xa, ya)) +
                          0.15f * std::sin(age * 18));
  s.color(l.halo);
  s.center(atlas::Bolt, xp, yp, 1.1f);
  s.color(col::white);
  s.center(atlas::Dot, xp, yp, 0.45f);
}

void Bolt::renderLight(Screen &s) {
  s.glow(xp, yp, 52 * scale(), LOOKS[int(style)].light);
}

void Bolt::spent() {
  const Look &l = LOOKS[int(style)];
  Spark::burst(*level, x, y, l.halo, 6, 60, true);
  if (style == BoltStyle::Arcane) {
    level->add<Shockwave>(x, y, 2, 12 * scale(), 0.22f, l.halo.alpha(0.8f),
                          false);
    level->add<Mark>(x, y + 6, 0.5f, atlas::pt::Rune, l.halo.alpha(0.7f),
                     scale(), age * 3);
  }
}
