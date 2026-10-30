#include "entity/projectile/ChakramDisc.hpp"

#include <cmath>

#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

ChakramDisc::ChakramDisc(double x, double y, double vx, double vy, float damage,
                         int pierce)
    : Projectile(x, y, vx, vy, 2.5f, damage, 45, pierce) {
  radius = 8;
}

void ChakramDisc::tick() {
  life -= DT;
  if (life <= 0)
    return remove();
  age += DT;
  Player &p = *level->player;
  double toX = p.x - x, toY = p.bodyY() - y;
  double d = std::sqrt(toX * toX + toY * toY);

  if (age > 0.35f) {
    if (age - DT <= 0.35f)
      hits.clear();
    if (d > 1e-3) {
      xa += (toX / d) * 780 * DT;
      ya += (toY / d) * 780 * DT;
    }
    if (d < 20 && age > 0.45f)
      return remove();
  }
  x += xa * DT;
  y += ya * DT;
  rot += 24 * DT;
  collide();
  if (rng.chance(0.4f) && Spark::alive < 600)
    level->add<Spark>(x, y, -xa * 0.1, -ya * 0.1, Color{0.55f, 0.85f, 1, 1},
                      0.25f, 0.35f, false);
}

void ChakramDisc::render(Screen &s) {
  float whirl = 0.5f + 0.25f * std::sin(rot * 2);
  s.ring(xp, yp, 8, Color{0.6f, 0.88f, 1, whirl});
  s.center(atlas::frontier(atlas::fr::Chakram), xp, yp, 1, rot);
}

void ChakramDisc::renderLight(Screen &s) {
  s.glow(xp, yp, 38, {0.10f, 0.22f, 0.35f, 1});
}

