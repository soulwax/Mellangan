#pragma once

#include "item/Upgrade.hpp"

class ChainLightning : public Upgrade {
public:
  ChainLightning();
  void tick(Mob &p) override;

private:
  float cd = 1;
};
