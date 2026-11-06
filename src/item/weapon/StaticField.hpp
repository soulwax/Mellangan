#pragma once

#include "item/Upgrade.hpp"

class StaticField : public Upgrade {
public:
  StaticField();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
