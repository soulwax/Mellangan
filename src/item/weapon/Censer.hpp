#pragma once

#include "item/Upgrade.hpp"

class Censer : public Upgrade {
public:
  Censer();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};

