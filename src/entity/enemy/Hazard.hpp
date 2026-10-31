#pragma once

#include "particle/Particle.hpp"

class Blast : public Particle {
public:
  float damage, radius = 38;
  bool hostile;

  Blast(double x, double y, float damage, bool hostile)
      : Particle(x, y, 0.85f), damage(damage), hostile(hostile) {}
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;
};

class FrostPatch : public Particle {
public:
  bool hostile;

  FrostPatch(double x, double y, bool hostile) : Particle(x, y, 3), hostile(hostile) {}
  int layer() const override { return 0; }
  void tick() override;
  void render(Screen &s) override;
};
