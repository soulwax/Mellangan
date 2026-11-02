#pragma once

#include "particle/Particle.hpp"

class Blast : public Particle {
public:
  float damage, radius = 38;
  bool hostile;
  Color tone{1, 0.55f, 0.15f, 1};
  const char *by = "a volatile blast";

  Blast(double x, double y, float damage, bool hostile)
      : Particle(x, y, 0.85f), damage(damage), hostile(hostile) {}
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;
};

class Bramble : public Particle {
public:
  explicit Bramble(double x, double y) : Particle(x, y, 3) {}
  int layer() const override { return 0; }
  void tick() override;
  void render(Screen &s) override;
};

class FrostPatch : public Particle {
public:
  bool hostile;
  Color tone{0.7f, 0.88f, 1, 1};

  FrostPatch(double x, double y, bool hostile) : Particle(x, y, 3), hostile(hostile) {}
  int layer() const override { return 0; }
  void tick() override;
  void render(Screen &s) override;
};
