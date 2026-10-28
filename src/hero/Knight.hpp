#pragma once

#include "hero/Hero.hpp"

class Knight : public Hero {
public:
  Knight();
  void tick(Player &p) const override;
  float guard(Player &p, float damage) const override;
};
