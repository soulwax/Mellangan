#pragma once

#include "hero/ExpansionHero.hpp"

class Warden : public ExpansionHero {
public:
  Warden();
  void equip(Mob &p) const override;
  void cast(Mob &p) const override;
};
