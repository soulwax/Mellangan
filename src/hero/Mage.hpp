#pragma once

#include "hero/Hero.hpp"

class Mage : public Hero {
public:
  Mage();
  void cast(Player &p) const override;
  float wisdom() const override { return 1.15f; }
};
