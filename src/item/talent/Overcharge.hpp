#pragma once

#include "item/Upgrade.hpp"

class Overcharge : public Upgrade {
public:
  Overcharge();
  void cast(Player &p) override;
};
