#pragma once

#include "item/Upgrade.hpp"

class Scattershot : public Upgrade {
public:
  Scattershot();
  void tick(Player &p) override;

private:
  float cd = 1;
};
