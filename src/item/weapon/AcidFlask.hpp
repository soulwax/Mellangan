#pragma once

#include "item/Upgrade.hpp"

class AcidFlask : public Upgrade {
public:
  AcidFlask();
  void tick(Player &p) override;

private:
  float cd = 1;
};
