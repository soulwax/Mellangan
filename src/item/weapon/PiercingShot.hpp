#pragma once

#include "item/Upgrade.hpp"

class PiercingShot : public Upgrade {
public:
  PiercingShot();
  void tick(Player &p) override;

private:
  float cd = 1;
};
