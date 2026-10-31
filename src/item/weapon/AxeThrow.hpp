#pragma once

#include "item/Upgrade.hpp"

class AxeThrow : public Upgrade {
public:
  AxeThrow();
  void tick(Mob &p) override;

private:
  float cd = 1;
};
