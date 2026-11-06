#pragma once

#include "item/Upgrade.hpp"

class FireBomb : public Upgrade {
public:
  FireBomb();
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;

private:
  float cd = 1.2f;
};
