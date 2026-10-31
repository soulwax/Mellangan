#pragma once

#include "item/Upgrade.hpp"

class VolatileMix : public Upgrade {
public:
  VolatileMix();
  void slay(Mob &p, Enemy &e) override;
};
