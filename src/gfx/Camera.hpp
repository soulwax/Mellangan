#pragma once

#include "Math.hpp"
#include "gfx/Screen.hpp"

struct Camera {
  double x = 0, y = 0;
  float shake = 0;

  void snap(double px, double py) { x = px, y = py; }
  void jolt(float strength) { shake = std::max(shake, strength); }

  void follow(double px, double py) {
    double k = damp(9, DT);
    x += (px - x) * k;
    y += (py - y) * k;
    shake *= std::exp(-10 * DT);
  }

  void aim(Screen &s) const {
    double xj = 0, yj = 0;
    if (shake > 0.05f)
      xj = rng.range(-1, 1) * shake, yj = rng.range(-1, 1) * shake;
    s.xOffset = int(std::floor(x + xj - s.vw / 2.0f));
    s.yOffset = int(std::floor(y + yj - s.vh / 2.0f));
  }
};
