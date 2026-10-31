#pragma once

#include "entity/Player.hpp"
#include "item/Upgrade.hpp"

class Passive : public Upgrade {
public:
  Passive(const char *name, Sprite icon, const char *desc, float Mob::*stat,
          float amount)
      : Upgrade(name, icon, {desc, desc, desc, desc, desc}, false), stat(stat),
        amount(amount) {}
  void levelUp(Mob &p) override;

private:
  float Mob::*stat;
  float amount;
};
