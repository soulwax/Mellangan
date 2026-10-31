#pragma once

#include "item/Upgrade.hpp"

class Riposte : public Upgrade {
public:
  Riposte();
  float guard(Mob &p, float damage) override;
};
