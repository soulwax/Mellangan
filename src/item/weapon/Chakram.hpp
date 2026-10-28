#pragma once

#include "item/Upgrade.hpp"

class Chakram : public Upgrade {
public:
  Chakram();
  void tick(Player &p) override;

private:
  float cd = 1;
};

