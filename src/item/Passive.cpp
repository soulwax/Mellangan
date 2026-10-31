#include "item/Passive.hpp"

#include "entity/Player.hpp"

void Passive::levelUp(Mob &p) {
  Upgrade::levelUp(p);
  p.*stat += amount;
  if (stat == &Mob::maxHp)
    p.hp += amount;
}
