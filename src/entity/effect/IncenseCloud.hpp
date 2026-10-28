#pragma once

#include "entity/Entity.hpp"

class IncenseCloud : public Entity {
public:
  IncenseCloud(double x, double y, double vx, double vy, float radius,
               float damage, float duration);
  int layer() const override { return 1; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  float damage, life, maxLife, timer = 0;
};

