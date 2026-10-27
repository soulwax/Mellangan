#pragma once

#include "particle/Particle.hpp"

class TextParticle : public Particle {
public:
  int value;
  Color color;

  inline static int alive = 0;

  TextParticle(double x, double y, int value, Color color)
      : Particle(x, y, 0.8f), value(value), color(color) {
    alive++;
  }
  ~TextParticle() override { alive--; }
  static void pop(Level &level, double x, double y, int value, Color c);

  void tick() override;
  void render(Screen &s) override;
};
