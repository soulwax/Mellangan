#include "item/Passive.hpp"

#include "entity/Player.hpp"

void Passive::levelUp(Player &p) {
  Upgrade::levelUp(p);
  p.*stat += amount;
  if (stat == &Player::maxHp)
    p.hp += amount;
}
