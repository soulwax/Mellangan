#pragma once

#include "hero/Hero.hpp"

class Knight : public Hero {
public:
  Knight();
  void cast(Player &p) const override;
  void tick(Player &p) const override;
  float guard(Player &p, float damage) const override;
};
