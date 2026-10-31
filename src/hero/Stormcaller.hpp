#pragma once

#include "hero/Hero.hpp"

class Stormcaller : public Hero {
public:
  Stormcaller();
  void cast(Mob &p) const override;
  void slay(Mob &p, Enemy &e) const override;
};
