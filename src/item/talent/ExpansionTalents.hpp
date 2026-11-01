#pragma once

#include "item/Upgrade.hpp"

class ExpansionTalent : public Upgrade {
public:
  explicit ExpansionTalent(int kind);
  void cast(Mob &p) override;
  void slay(Mob &p, Enemy &e) override;
  void tick(Mob &p) override;

  int kind;
  float timer = 3;
};
