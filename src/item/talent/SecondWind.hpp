#pragma once

#include "item/Upgrade.hpp"

class SecondWind : public Upgrade {
public:
  SecondWind();
  void slay(Player &p, Enemy &e) override;
};
