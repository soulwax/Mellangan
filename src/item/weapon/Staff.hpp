#pragma once

#include "item/Upgrade.hpp"

class Staff : public Upgrade {
public:
  Staff();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 0.4f;
};
