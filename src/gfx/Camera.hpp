#pragma once

#include "Math.hpp"
#include "gfx/Screen.hpp"

struct Camera {
  double x = 0, y = 0, xLast = 0, yLast = 0;
  float shake = 0, scale = 1;

  void snap(double px, double py) { x = xLast = px, y = yLast = py; }
  void remember() { xLast = x, yLast = y; }
  void jolt(float strength) { shake = std::max(shake, strength); }

  void follow(double px, double py) {
    double k = damp(9, DT);
    x += (px - x) * k;
    y += (py - y) * k;
    shake *= std::exp(-10 * DT);
  }

  void aim(Screen &s, float a = 1) const {
    double xj = 0, yj = 0;
    if (shake > 0.05f)
      xj = rng.range(-1, 1) * shake * scale, yj = rng.range(-1, 1) * shake * scale;
    double xc = xLast + (x - xLast) * a, yc = yLast + (y - yLast) * a;
    s.xOffset = int(std::floor(xc + xj - s.vw / 2.0f));
    s.yOffset = int(std::floor(yc + yj - s.vh / 2.0f));
  }
};
