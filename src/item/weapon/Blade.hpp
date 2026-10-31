#pragma once

#include "item/Upgrade.hpp"

class Blade : public Upgrade {
public:
  Blade();
  void tick(Mob &p) override;

private:
  float cd = 0.6f;
};
