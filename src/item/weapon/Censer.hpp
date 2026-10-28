#pragma once

#include "item/Upgrade.hpp"

class Censer : public Upgrade {
public:
  Censer();
  void tick(Player &p) override;

private:
  float cd = 1;
};

