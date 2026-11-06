#pragma once

#include "item/Upgrade.hpp"

class Scattershot : public Upgrade {
public:
  Scattershot();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
