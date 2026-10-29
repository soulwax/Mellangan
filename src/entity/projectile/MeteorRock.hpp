#pragma once

#include "entity/Entity.hpp"

class MeteorRock : public Entity {
public:
  MeteorRock(double x, double y, float damage, float radius, float burn)
      : damage(damage), radius(radius), burn(burn) {
    this->x = x, this->y = y;
  }
  int layer() const override { return 2; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  static constexpr float FALL = 0.65f, DX = -80, DY = -190;
  float damage, radius, burn, t = 0;

  float drop() const { return 1 - t * t; }
  void impact();
};
