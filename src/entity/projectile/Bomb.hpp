#pragma once

#include "entity/Entity.hpp"
#include "entity/effect/FirePool.hpp"

class Bomb : public Entity {
public:
  Bomb(double x0, double y0, double x1, double y1, float damage,
       float poolRadius, float poolDamage, float poolTime,
       PoolStyle style = PoolStyle::Fire)
      : x0(x0), y0(y0), x1(x1), y1(y1), damage(damage), poolRadius(poolRadius),
        poolDamage(poolDamage), poolTime(poolTime), style(style) {
    x = x0, y = y0;
  }
  bool hostile = false;
  int layer() const override { return 2; }
  void tick() override;
  void render(Screen &s) override;

private:
  double x0, y0, x1, y1;
  float damage, poolRadius, poolDamage, poolTime, t = 0;
  PoolStyle style;
  void burst();
};
