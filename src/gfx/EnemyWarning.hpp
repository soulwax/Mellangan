#pragma once

#include "Math.hpp"
#include "gfx/Screen.hpp"

inline void enemyWarning(Screen &s, double x, double y, float radius,
                         float alpha, float phase) {
  Color c{1, 0.18f, 0.12f, alpha};
  s.color(c);
  s.ring(x, y, radius);
  s.color(col::white);
  for (int i = 0; i < 4; i++) {
    float a = PI * 0.5f * i;
    double dx = std::cos(a), dy = std::sin(a);
    s.line(x + dx * (radius - 3), y + dy * (radius - 3),
           x + dx * (radius + 4), y + dy * (radius + 4), 2, c);
  }
}
