#pragma once

#include "hero/Hero.hpp"

class Alchemist : public Hero {
public:
  Alchemist();
  void cast(Mob &p) const override;
  void slay(Mob &p, Enemy &e) const override;
};
