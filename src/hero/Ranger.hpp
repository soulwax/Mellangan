#pragma once

#include "hero/Hero.hpp"

class Ranger : public Hero {
public:
  Ranger();
  void cast(Player &p) const override;
  float guard(Player &p, float damage) const override;
};
