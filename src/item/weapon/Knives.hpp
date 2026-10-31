#pragma once

#include "item/Upgrade.hpp"

class Knives : public Upgrade {
public:
  float angle = 0;

  Knives();
  int count() const { return 2 + at(level, 2) + at(level, 3) + at(level, 4); }
  float damage() const { return 9.0f + 4 * (at(level, 3) + at(level, 5)); }
  void levelUp(Mob &p) override;
  void tick(Mob &p) override;

private:
  int spawned = 0;
};
