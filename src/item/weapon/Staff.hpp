#pragma once

#include "item/Upgrade.hpp"

class Staff : public Upgrade {
public:
  Staff();
  void tick(Mob &p) override;

private:
  float cd = 0.4f;
};
