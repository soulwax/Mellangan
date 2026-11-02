#include "entity/Player.hpp"

#include <cstring>

#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "item/loot/Equip.hpp"

bool Player::fits(const Item &item) const {
  const char *klass = item.def().klass;
  return !klass || (hero && std::strcmp(klass, hero->name) == 0);
}

void Player::wear(int slot, const Item &item) {
  gear[slot] = item, worn[slot] = true, gearLevels[slot] = 0;
  applyItem(*this, item, 1);
  if (const char *grants = item.grants())
    if (Upgrade *up = upgrade(grants)) {
      int before = up->level;
      while (up->level < item.tier + 1 && !up->maxed())
        up->levelUp(*this);
      gearLevels[slot] = up->level - before;
    }
}

void Player::shed(int slot) {
  const Item &item = gear[slot];
  applyItem(*this, item, -1);
  if (const char *grants = item.grants())
    if (Upgrade *up = upgrade(grants)) {
      up->level = std::max(0, up->level - gearLevels[slot]);
      if (up->level == 0)
        up->forget(*this);
    }
  worn[slot] = false, gearLevels[slot] = 0;
}

bool Player::equip(size_t bagIndex) {
  if (bagIndex >= bag.size() || !fits(bag[bagIndex]))
    return false;
  Item item = bag[bagIndex];
  int slot = item.gearSlot();
  if (slot == GearCharm && worn[GearCharm] && !worn[GearCharm2])
    slot = GearCharm2;
  if (worn[slot]) {
    bag[bagIndex] = gear[slot];
    shed(slot);
  } else {
    bag.erase(bag.begin() + long(bagIndex));
  }
  wear(slot, item);
  return true;
}

bool Player::unequip(int slot) {
  if (!worn[slot] || int(bag.size()) >= BAG)
    return false;
  bag.push_back(gear[slot]);
  shed(slot);
  return true;
}

Item Player::strip(int slot) {
  Item item = gear[slot];
  if (worn[slot])
    shed(slot);
  return item;
}
