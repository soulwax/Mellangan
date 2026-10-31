#pragma once

#include "item/Upgrade.hpp"

class WhirlingEdge : public Upgrade {
public:
  WhirlingEdge();
  void tick(Mob &p) override;

private:
  float cd = 1;
};
