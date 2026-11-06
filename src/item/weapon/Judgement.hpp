#pragma once

#include "item/Upgrade.hpp"

class Judgement : public Upgrade {
public:
  Judgement();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
