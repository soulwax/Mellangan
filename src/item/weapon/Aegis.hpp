#pragma once

#include "item/Upgrade.hpp"

class Aegis : public Upgrade {
public:
  Aegis();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
