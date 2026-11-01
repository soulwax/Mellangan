#pragma once

#include "hero/ExpansionHero.hpp"

class Tinkerer : public ExpansionHero {
public:
  Tinkerer();
  void cast(Mob &p) const override;
  void slay(Mob &p, Enemy &e) const override;
};
