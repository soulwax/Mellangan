#pragma once

#include "item/Upgrade.hpp"

class Aegis : public Upgrade {
public:
  Aegis();
  void tick(Player &p) override;

private:
  float cd = 1;
};
