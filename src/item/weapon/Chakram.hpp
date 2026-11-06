#pragma once

#include "item/Upgrade.hpp"

class Chakram : public Upgrade {
public:
  Chakram();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};

