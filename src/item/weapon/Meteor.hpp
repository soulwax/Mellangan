#pragma once

#include "item/Upgrade.hpp"

class Meteor : public Upgrade {
public:
  Meteor();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1;
};
