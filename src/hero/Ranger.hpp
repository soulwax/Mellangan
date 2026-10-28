#pragma once

#include "hero/Hero.hpp"

class Ranger : public Hero {
public:
  Ranger();
  float guard(Player &p, float damage) const override;
};
