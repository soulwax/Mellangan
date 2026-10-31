#pragma once

#include "item/Upgrade.hpp"

class Meteor : public Upgrade {
public:
  Meteor();
  void tick(Mob &p) override;

private:
  float cd = 1;
};
