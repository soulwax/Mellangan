#include "entity/pickup/Pickup.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"

void Pickup::tick() {
  t += DT;
  z = -std::sin(t * 4 + x) * 1.5;
  Player &p = *level->player;
  if (p.dead())
    return;
  double xd = p.x - x, yd = p.bodyY() - y;
  double d = std::sqrt(xd * xd + yd * yd);
  if (!magnet) {
    if (d < reach())
      magnet = true, speed = -60,
      swirl = (rng.chance(0.5f) ? 1 : -1) * rng.range(0.5f, 1.0f);
    return;
  }
  speed += 650 * DT;
  if (d < 5 || d < speed * DT)
    return take(p), remove();
  double turn = swirl * std::min(1.0, d / 70), c = std::cos(turn),
         s = std::sin(turn), ux = xd / d, uy = yd / d;
  x += (ux * c - uy * s) * speed * DT;
  y += (ux * s + uy * c) * speed * DT;
}

float Pickup::reach() const { return level->player->magnet; }
