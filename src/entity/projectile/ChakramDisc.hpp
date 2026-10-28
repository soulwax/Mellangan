#pragma once

#include "entity/projectile/Projectile.hpp"

class ChakramDisc : public Projectile {
public:
  float age = 0, rot = 0;

  ChakramDisc(double x, double y, double vx, double vy, float damage,
              int pierce);
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;
};
