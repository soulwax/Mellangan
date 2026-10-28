#pragma once

#include "item/Upgrade.hpp"

class ChainLightning : public Upgrade {
public:
  ChainLightning();
  void tick(Player &p) override;

private:
  float cd = 1;
};
