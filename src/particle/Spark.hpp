#pragma once

#include "particle/Particle.hpp"

class Spark : public Particle {
public:
  Color color;
  float size;
  bool glow;

  inline static int alive = 0;

  Spark(double x, double y, double xa, double ya, Color color, float life,
        float size, bool glow)
      : Particle(x, y, life), color(color), size(size), glow(glow) {
    this->xa = xa, this->ya = ya, sprite = atlas::Dot, alive++;
  }
  ~Spark() override { alive--; }
  static void burst(Level &level, double x, double y, Color c, int n,
                    float speed, bool glow);

  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;
};
