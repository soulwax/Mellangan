#pragma once

#include "particle/Particle.hpp"

class Shockwave : public Particle {
public:
  float from, to;
  Color color;
  bool glow;
  int spokes = 0;

  Shockwave(double x, double y, float from, float to, float life, Color color,
            bool glow = true)
      : Particle(x, y, life), from(from), to(to), color(color), glow(glow) {}
  float radius() const;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;
};
