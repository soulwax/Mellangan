#pragma once

#include "item/Upgrade.hpp"

class WraithBolts : public Upgrade {
public:
  WraithBolts();
  void tick(Mob &p) override;

private:
  float cd = 1;
};
