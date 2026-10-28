#pragma once

#include "item/Upgrade.hpp"

class Radiance : public Upgrade {
public:
  Radiance();
  void tick(Player &p) override;

private:
  float cd = 1;
};
