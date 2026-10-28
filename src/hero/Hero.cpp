#include "hero/Hero.hpp"

#include "entity/Player.hpp"
#include "hero/Alchemist.hpp"
#include "hero/Knight.hpp"
#include "hero/Mage.hpp"
#include "hero/Ranger.hpp"
#include "hero/Reaper.hpp"
#include "hero/Stormcaller.hpp"

namespace {
const Mage mage;
const Ranger ranger;
const Knight knight;
const Alchemist alchemist;
const Stormcaller stormcaller;
const Reaper reaper;
} // namespace

const Hero *const HEROES[HERO_COUNT] = {&mage,      &ranger,      &knight,
                                        &alchemist, &stormcaller, &reaper};

void Hero::equip(Player &p) const {
  p.tile = tile;
  p.hp = p.maxHp += hp;
  p.speed *= speed;
  p.might += might, p.haste += haste, p.magnet += magnet;
  p.armor += armor, p.regen += regen, p.luck += luck;
}

const Hero &heroOf(const Codex &c) {
  return c.start >= 0 && c.start < HERO_COUNT && HEROES[c.start]->met(c)
             ? *HEROES[c.start]
             : *HEROES[0];
}
