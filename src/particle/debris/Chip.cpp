#include "particle/debris/Chip.hpp"

Chip::Chip(double x, double y, double z, int icon, Color color, double speed)
    : Debris(x, y, z), rot(rng.range(0, TAU)), spin(rng.range(-0.4f, 0.4f)) {
  this->icon = icon;
  this->color = color;
  life = rng.irange(35, 55);
  bounce = 0.35;
  xa *= speed, ya *= speed, za *= speed * 0.6;
}

void Chip::tick() {
  if (z > 1)
    rot += spin;
  Debris::tick();
}

void Chip::render(Screen &s) {
  float fade = std::min(1.0f, life / 15.0f);
  s.particle(icon, xp, yp, color.alpha(color.a * fade), 1, rot);
}
