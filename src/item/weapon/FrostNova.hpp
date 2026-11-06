#pragma once

#include "item/Upgrade.hpp"

class FrostNova : public Upgrade {
public:
  FrostNova();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
