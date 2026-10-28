#pragma once

#include "item/Upgrade.hpp"

class FrostNova : public Upgrade {
public:
  FrostNova();
  void tick(Player &p) override;

private:
  float cd = 1;
};
