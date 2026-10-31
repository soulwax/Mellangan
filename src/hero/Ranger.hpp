#pragma once

#include "hero/Hero.hpp"

class Ranger : public Hero {
public:
  Ranger();
  void cast(Mob &p) const override;
  float guard(Mob &p, float damage) const override;
};
