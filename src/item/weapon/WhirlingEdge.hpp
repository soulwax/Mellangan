#pragma once

#include "item/Upgrade.hpp"

class WhirlingEdge : public Upgrade {
public:
  WhirlingEdge();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
