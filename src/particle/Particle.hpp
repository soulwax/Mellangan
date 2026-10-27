#pragma once

#include "SpriteObject.hpp"

class Particle : public SpriteObject {
public:
  float life, maxLife;

  Particle(double x, double y, float life) : life(life), maxLife(life) {
    this->x = x, this->y = y;
  }
  int layer() const override { return 2; }
  void tick() override {
    if ((life -= DT) <= 0)
      remove();
  }

protected:
  float frac() const { return life / maxLife; }
};
