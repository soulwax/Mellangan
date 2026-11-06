#pragma once

#include "item/Upgrade.hpp"

class ChainLightning : public Upgrade {
public:
  ChainLightning();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
