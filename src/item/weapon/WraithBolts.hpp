#pragma once

#include "item/Upgrade.hpp"

class WraithBolts : public Upgrade {
public:
  WraithBolts();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
