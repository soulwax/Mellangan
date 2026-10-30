#pragma once

#include "entity/projectile/Projectile.hpp"

enum class ArrowStyle { Hunter, Piercing, Scatter };

class Arrow : public Projectile {
public:
  Arrow(double x, double y, double xa, double ya, float damage, int pierce,
        ArrowStyle style = ArrowStyle::Hunter)
      : Projectile(x, y, xa, ya, 1.2f, damage, 35, pierce), style(style) {
    radius = 5;
  }
  void tick() override;
  void render(Screen &s) override;

private:
  ArrowStyle style;
  size_t pierced = 0;
};
