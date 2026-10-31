#pragma once

#include "item/Upgrade.hpp"

class Censer : public Upgrade {
public:
  Censer();
  void tick(Mob &p) override;

private:
  float cd = 1;
};

