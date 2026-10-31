#pragma once

#include "hero/Hero.hpp"

class Reaper : public Hero {
public:
  Reaper();
  void cast(Mob &p) const override;
  void slay(Mob &p, Enemy &e) const override;
};
