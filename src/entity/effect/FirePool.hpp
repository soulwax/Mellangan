#pragma once

#include "entity/Entity.hpp"

enum class PoolStyle { Fire, Acid };

class FirePool : public Entity {
public:
  FirePool(double x, double y, float radius, float damage, float duration,
           PoolStyle style = PoolStyle::Fire)
      : damage(damage), life(duration), maxLife(duration), style(style) {
    this->x = x, this->y = y, this->radius = radius;
  }
  int layer() const override { return 0; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  float damage, life, maxLife, timer = 0;
  PoolStyle style;
};
