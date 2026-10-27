#pragma once

#include "item/Upgrade.hpp"

class Staff : public Upgrade {
public:
  Staff();
  void tick(Player &p) override;

private:
  float cd = 0.4f;
};
