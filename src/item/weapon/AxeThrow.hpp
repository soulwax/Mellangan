#pragma once

#include "item/Upgrade.hpp"

class AxeThrow : public Upgrade {
public:
  AxeThrow();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
