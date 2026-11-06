#pragma once

#include "item/Upgrade.hpp"

class Bow : public Upgrade {
public:
  Bow();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 0.5f;
};
