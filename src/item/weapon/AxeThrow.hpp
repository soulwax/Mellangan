#pragma once

#include "item/Upgrade.hpp"

class AxeThrow : public Upgrade {
public:
  AxeThrow();
  void tick(Player &p) override;

private:
  float cd = 1;
};
