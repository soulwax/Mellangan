#pragma once

#include "hero/ExpansionHero.hpp"

class Corsair : public ExpansionHero {
public:
  Corsair();
  void cast(Mob &p) const override;
  float edge() const override { return 2.2f; }
};
