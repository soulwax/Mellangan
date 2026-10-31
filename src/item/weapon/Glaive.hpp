#pragma once

#include "item/Upgrade.hpp"

class Glaive : public Upgrade {
public:
  Glaive();
  void tick(Mob &p) override;

private:
  float cd = 1;
};

