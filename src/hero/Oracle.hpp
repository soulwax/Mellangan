#pragma once

#include "hero/ExpansionHero.hpp"

class Oracle : public ExpansionHero {
public:
  Oracle();
  void cast(Mob &p) const override;
  float wisdom() const override { return 1.15f; }
};
