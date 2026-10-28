#pragma once

#include "item/Upgrade.hpp"

class Harvest : public Upgrade {
public:
  Harvest();
  void slay(Player &p, Enemy &e) override;
};
