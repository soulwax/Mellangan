#pragma once

#include "item/Upgrade.hpp"

class Transmute : public Upgrade {
public:
  Transmute();
  void tick(Player &p) override;

private:
  float cd = 1;
};
