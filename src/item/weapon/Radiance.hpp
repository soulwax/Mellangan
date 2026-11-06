#pragma once

#include "item/Upgrade.hpp"

class Radiance : public Upgrade {
public:
  Radiance();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
