#pragma once

#include "item/Upgrade.hpp"

class Scattershot : public Upgrade {
public:
  Scattershot();
  void tick(Mob &p) override;

private:
  float cd = 1;
};
