#pragma once

#include "item/Upgrade.hpp"

class Retribution : public Upgrade {
public:
  Retribution();
  float guard(Player &p, float damage) override;
};
