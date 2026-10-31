#pragma once

#include "item/Upgrade.hpp"

class Radiance : public Upgrade {
public:
  Radiance();
  void tick(Mob &p) override;

private:
  float cd = 1;
};
