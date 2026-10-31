#pragma once

#include "item/Upgrade.hpp"

class Harvest : public Upgrade {
public:
  Harvest();
  void slay(Mob &p, Enemy &e) override;
};
