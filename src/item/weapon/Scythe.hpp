#pragma once

#include "item/Upgrade.hpp"

class Scythe : public Upgrade {
public:
  Scythe();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
