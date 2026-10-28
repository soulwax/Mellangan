#pragma once

#include "item/Upgrade.hpp"

class VolatileMix : public Upgrade {
public:
  VolatileMix();
  void slay(Player &p, Enemy &e) override;
};
