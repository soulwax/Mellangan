#pragma once

#include "item/Upgrade.hpp"

class Glaive : public Upgrade {
public:
  Glaive();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};

