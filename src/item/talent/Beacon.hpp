#pragma once

#include "item/Upgrade.hpp"

class Beacon : public Upgrade {
public:
  Beacon();
  void cast(Player &p) override;
};
