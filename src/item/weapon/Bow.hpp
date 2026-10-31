#pragma once

#include "item/Upgrade.hpp"

class Bow : public Upgrade {
public:
  Bow();
  void tick(Mob &p) override;

private:
  float cd = 0.5f;
};
