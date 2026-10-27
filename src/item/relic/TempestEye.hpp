#pragma once

#include "item/Upgrade.hpp"

class TempestEye : public Upgrade {
public:
  TempestEye();
  void tick(Player &p) override;

private:
  float cd = 0.5f;
};
