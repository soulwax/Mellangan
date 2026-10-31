#pragma once

#include "item/Upgrade.hpp"

class SoulSiphon : public Upgrade {
public:
  SoulSiphon();
  void tick(Mob &p) override;

private:
  float cd = 1;
};
