#pragma once

#include "item/Upgrade.hpp"

class SecondWind : public Upgrade {
public:
  SecondWind();
  void slay(Mob &p, Enemy &e) override;
};
