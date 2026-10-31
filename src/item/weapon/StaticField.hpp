#pragma once

#include "item/Upgrade.hpp"

class StaticField : public Upgrade {
public:
  StaticField();
  void tick(Mob &p) override;

private:
  float cd = 1;
};
