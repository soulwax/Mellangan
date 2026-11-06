#pragma once

#include "item/Upgrade.hpp"

class StormRod : public Upgrade {
public:
  StormRod();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1.5f;
};
