#pragma once

#include "item/Upgrade.hpp"

class SoulSiphon : public Upgrade {
public:
  SoulSiphon();
  void tick(Player &p) override;

private:
  float cd = 1;
};
