#include "particle/Mark.hpp"

void Mark::render(Screen &s) {
  float fade = std::min({1.0f, frac() * 2.5f, (maxLife - life) * 10});
  Color c = color.alpha(color.a * fade);
  if (icon >= 0)
    return s.particle(icon, xp, yp, c, scale, rot);
  s.color(c);
  s.center(art, xp, yp, scale);
  s.color(col::white);
}
