#pragma once

#include "item/Upgrade.hpp"

class Flurry : public Upgrade {
public:
  Flurry();
  void tick(Mob &p) override;

private:
  float cd = 1, gap = 0, dmg = 0, side = 1;
  int pending = 0;
};
