#pragma once

#include "hero/ExpansionHero.hpp"

class Oracle : public ExpansionHero {
public:
  Oracle();
  void cast(Mob &p) const override;
  void slay(Mob &p, Enemy &e) const override;
};
