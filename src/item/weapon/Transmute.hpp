#pragma once

#include "item/Upgrade.hpp"

class Transmute : public Upgrade {
public:
  Transmute();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
