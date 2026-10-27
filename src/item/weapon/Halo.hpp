#pragma once

#include "item/Upgrade.hpp"

class Halo : public Upgrade {
public:
  float pulse = 0;

  Halo();
  float range() const { return 30.0f + 7 * (level - 1); }
  void levelUp(Player &p) override;
  void tick(Player &p) override;

private:
  float timer = 0;
};
