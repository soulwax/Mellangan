#pragma once

#include "item/Upgrade.hpp"

class FrostNova : public Upgrade {
public:
  FrostNova();
  void tick(Mob &p) override;

private:
  float cd = 1;
};
