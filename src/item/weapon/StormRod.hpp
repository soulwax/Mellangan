#pragma once

#include "item/Upgrade.hpp"

class StormRod : public Upgrade {
public:
  StormRod();
  void tick(Player &p) override;

private:
  float cd = 1.5f;
};
