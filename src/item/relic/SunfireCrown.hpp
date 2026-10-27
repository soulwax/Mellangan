#pragma once

#include "item/Upgrade.hpp"

class SunfireCrown : public Upgrade {
public:
  SunfireCrown();
  void tick(Player &p) override;

private:
  float cd = 1;
};
