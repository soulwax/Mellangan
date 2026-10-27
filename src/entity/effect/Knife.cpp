#include "entity/effect/Knife.hpp"

#include "entity/Player.hpp"
#include "item/weapon/Knives.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

void Knife::tick() {
  angle = knives.angle + TAU * index / knives.count();
  x = player.x + std::cos(angle) * 34;
  y = player.bodyY() + std::sin(angle) * 34;
  float dmg = knives.damage() * player.might;
  level->enemiesNear(x, y, 32, [&](Enemy &e) {
    double xd = e.x - x, yd = e.bodyY() - y;
    float rr = 6 + e.radius;
    if (e.knifeCooldown > 0 || xd * xd + yd * yd >= rr * rr)
      return;
    e.hurt(dmg, e.x - player.x, e.y - player.y, 70);
    e.knifeCooldown = 0.45f;
    Spark::burst(*level, x, y, col::white, 2, 50, false);
  });
}

void Knife::render(Screen &s) {
  s.center(atlas::tile(atlas::t::Dagger), xp, yp, 1, angle + PI);
}

void Knife::renderLight(Screen &s) {
  s.glow(xp, yp, 30, {0.08f, 0.08f, 0.12f, 1});
}
