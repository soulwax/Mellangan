#include "hero/Hero.hpp"

#include <cstring>

#include "entity/Player.hpp"
#include "hero/Alchemist.hpp"
#include "hero/Duelist.hpp"
#include "hero/Knight.hpp"
#include "hero/Mage.hpp"
#include "hero/Priestess.hpp"
#include "hero/Ranger.hpp"
#include "hero/Reaper.hpp"
#include "hero/Stormcaller.hpp"
#include "hero/ExpansionHero.hpp"

namespace {
const Mage mage;
const Ranger ranger;
const Knight knight;
const Alchemist alchemist;
const Stormcaller stormcaller;
const Reaper reaper;
const Priestess priestess;
const Duelist duelist;
const ExpansionHero warden{0},tinkerer{1},oracle{2},corsair{3};
} // namespace

const Hero *const HEROES[HERO_COUNT] = {
    &mage,        &ranger, &knight,    &alchemist,
    &stormcaller, &reaper, &priestess, &duelist,
    &warden,&tinkerer,&oracle,&corsair};

void Hero::equip(Player &p) const {
  p.tile = tile;
  p.hp = p.maxHp += hp;
  p.speed *= speed;
  p.might += might, p.haste += haste, p.magnet += magnet;
  p.armor += armor, p.regen += regen, p.luck += luck;
}

bool Hero::favours(const char *upgrade) const {
  for (const char *a : affinity)
    if (std::strcmp(a, upgrade) == 0)
      return true;
  return false;
}

const Hero &heroOf(const Codex &c) {
  return c.start >= 0 && c.start < HERO_COUNT && HEROES[c.start]->met(c)
             ? *HEROES[c.start]
             : *HEROES[0];
}
