#pragma once

#include "item/Upgrade.hpp"

class Meteor : public Upgrade {
public:
  Meteor();
  void tick(Player &p) override;

private:
  float cd = 1;
};
