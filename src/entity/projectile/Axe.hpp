#pragma once

#include "entity/projectile/Projectile.hpp"

class Axe : public Projectile {
public:
  Axe(double x, double y, double xa, double ya, float damage, int pierce,
      float spin);
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  float rot = 0, spin;
  size_t chopped = 0;
};
