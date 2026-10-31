#pragma once

#include "item/Upgrade.hpp"

class Retribution : public Upgrade {
public:
  Retribution();
  float guard(Mob &p, float damage) override;
};
