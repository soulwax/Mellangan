#include "particle/Drift.hpp"

Drift::Drift(double x, double y, double xa, double ya, int icon, Color color,
             float life)
    : Particle(x, y, life), icon(icon), color(color), rot(rng.range(0, TAU)) {
  this->xa = xa, this->ya = ya;
  alive++;
}

void Drift::tick() {
  Particle::tick();
  double k = std::exp(-drag * DT);
  xa *= k, ya = ya * k + gravity * DT;
  float age = maxLife - life;
  x += (xa + std::cos(age * 5 + rot) * sway) * DT;
  y += ya * DT;
  rot += spin * DT;
}

void Drift::render(Screen &s) {
  float age = maxLife - life;
  float a = std::min({1.0f, frac() * 2.5f, age * 10});
  s.particle(icon, xp, yp, color.alpha(color.a * a),
             std::max(0.25f, scale + grow * age), rot);
}
