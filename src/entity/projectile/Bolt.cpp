#include "entity/projectile/Bolt.hpp"

#include "entity/enemy/Enemy.hpp"
#include "entity/projectile/Projectile.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

Bolt::Bolt(double x, double y, double xa, double ya, float damage, int pierce,
           entt::entity target)
    : Projectile(x, y, xa, ya, 1.6f, damage, 45, pierce), target(target) {}

void Bolt::tick() {
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
  if (rng.chance(0.5f)) {
    float dir = rng.range(0, TAU);
    level->add<Spark>(x, y, xa * -0.1 + std::cos(dir) * 8,
                      ya * -0.1 + std::sin(dir) * 8, Color{0.4f, 0.75f, 1, 1},
                      0.25f, 0.35f, false);
  }
  collide();
}

void Bolt::render(Screen &s) {
  s.color({0.45f, 0.8f, 1, 1});
  s.center(atlas::Bolt, xp, yp, 1.1f);
  s.color(col::white);
  s.center(atlas::Dot, xp, yp, 0.45f);
}

void Bolt::renderLight(Screen &s) {
  s.glow(xp, yp, 52, {0.06f, 0.20f, 0.36f, 1});
}

void Bolt::spent() {
  Spark::burst(*level, x, y, {0.45f, 0.8f, 1, 1}, 6, 60, true);
}
