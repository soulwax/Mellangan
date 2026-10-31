#include "entity/effect/Knife.hpp"

#include "entity/Player.hpp"
#include "item/weapon/Knives.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

void Knife::tick() {
  if (!level->mob(ownerId))
    return remove();
  angle = knives.angle + TAU * index / knives.count();
  x = owner.x + std::cos(angle) * 34;
  y = owner.bodyY() + std::sin(angle) * 34;
  float dmg = knives.damage() * owner.might * knives.power;
  level->foes((source && source->hostile()), x, y, 32, [&](Mob &e) {
    double xd = e.x - x, yd = e.bodyY() - y;
    float rr = 6 + e.radius;
    if (e.knifeCooldown > 0 || xd * xd + yd * yd >= rr * rr)
      return;
    e.strike(dmg, e.x - owner.x, e.y - owner.y, 70, source);
    e.knifeCooldown = 0.45f;
    Spark::burst(*level, x, y, col::white, 2, 50, false);
  });
}

void Knife::render(Screen &s) {
  double cy = owner.bodyY();
  for (int i = 2; i >= 1; i--) {
    float a = angle - i * 0.22f;
    s.color(Color{0.8f, 0.88f, 1, 0.18f * float(3 - i)});
    s.center(atlas::tile(atlas::t::Dagger), owner.x + std::cos(a) * 34,
             cy + std::sin(a) * 34, 1, a + PI);
  }
  s.color(col::white);
  s.center(atlas::tile(atlas::t::Dagger), xp, yp, 1, angle + PI);
}

void Knife::renderLight(Screen &s) {
  s.glow(xp, yp, 30, {0.08f, 0.08f, 0.12f, 1});
}
