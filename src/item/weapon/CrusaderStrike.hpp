#pragma once

#include "item/Upgrade.hpp"

class CrusaderStrike : public Upgrade {
public:
  CrusaderStrike();
  void tick(Mob &p) override;

private:
  float cd = 1;
};
