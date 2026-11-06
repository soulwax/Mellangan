#pragma once

#include "item/Upgrade.hpp"

class Knives : public Upgrade {
public:
  float angle = 0, ring = 0;

  Knives();
  int count() const { return count(level); }
  float damage() const { return damage(level); }
  static int count(int lvl) { return 2 + at(lvl, 2) + at(lvl, 3) + at(lvl, 4); }
  static float damage(int lvl) { return 9.0f + 4 * (at(lvl, 3) + at(lvl, 5)); }
  void levelUp(Mob &p) override;
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  int spawned = 0;
};
