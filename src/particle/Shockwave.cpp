#include "particle/Shockwave.hpp"

float Shockwave::radius() const {
  float k = 1 - frac();
  return from + (to - from) * (1 - (1 - k) * (1 - k) * (1 - k));
}

void Shockwave::render(Screen &s) {
  float r = radius(), a = color.a * std::pow(frac(), 0.7f);
  s.ring(xp, yp, r, color.alpha(a), 2);
  s.ring(xp, yp, r - 4, color.alpha(a * 0.35f));
  for (int i = 0; i < spokes; i++) {
    float t = TAU * i / spokes;
    double cx = std::cos(t), cy = std::sin(t);
    s.line(xp + cx * r * 0.55, yp + cy * r * 0.55, xp + cx * r * 0.92,
           yp + cy * r * 0.92, 2, color.alpha(a * 0.6f));
  }
}

void Shockwave::renderLight(Screen &s) {
  if (glow)
    s.glow(xp, yp, radius() * 2.2f, color * (0.14f * frac()));
}
