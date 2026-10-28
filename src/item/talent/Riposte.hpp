#pragma once

#include "item/Upgrade.hpp"

class Riposte : public Upgrade {
public:
  Riposte();
  float guard(Player &p, float damage) override;
};
