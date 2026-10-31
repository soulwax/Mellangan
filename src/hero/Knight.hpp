#pragma once

#include "hero/Hero.hpp"

class Knight : public Hero {
public:
  Knight();
  void cast(Mob &p) const override;
  void tick(Mob &p) const override;
  float guard(Mob &p, float damage) const override;
};
