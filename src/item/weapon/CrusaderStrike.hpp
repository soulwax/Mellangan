#pragma once

#include "item/Upgrade.hpp"

class CrusaderStrike : public Upgrade {
public:
  CrusaderStrike();
  void tick(Player &p) override;

private:
  float cd = 1;
};
