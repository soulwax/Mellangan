#pragma once

#include "hero/Hero.hpp"

class Duelist : public Hero {
public:
  Duelist();
  void cast(Mob &p) const override;
  float edge() const override { return 2.6f; }
};
