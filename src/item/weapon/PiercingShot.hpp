#pragma once

#include "item/Upgrade.hpp"

class PiercingShot : public Upgrade {
public:
  PiercingShot();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
