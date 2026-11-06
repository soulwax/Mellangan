#pragma once

#include "item/Upgrade.hpp"

class Halo : public Upgrade {
public:
  float pulse = 0;

  Halo();
  float range() const { return range(level); }
  static float range(int lvl) { return 30.0f + 7 * (lvl - 1); }
  void levelUp(Mob &p) override;
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float timer = 0;
  bool hum = false;
};
