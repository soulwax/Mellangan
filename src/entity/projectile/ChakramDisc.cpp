#include "entity/projectile/ChakramDisc.hpp"

#include <cmath>

#include "entity/Player.hpp"
#include "level/Level.hpp"

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
}

void ChakramDisc::render(Screen &s) {
  s.center(atlas::frontier(atlas::fr::Chakram), xp, yp, 1, rot);
}

void ChakramDisc::renderLight(Screen &s) {
  s.glow(xp, yp, 38, {0.10f, 0.22f, 0.35f, 1});
}

