#pragma once

#include "hero/Hero.hpp"

class Mage : public Hero {
public:
  Mage();
  float wisdom() const override { return 1.15f; }
};
