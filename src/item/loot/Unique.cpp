#include "item/loot/Unique.hpp"

#include <vector>

#include "item/loot/Item.hpp"

namespace {

using S = Stat;

int affixFor(Stat stat) {
  for (int i = 0; i < AFFIX_COUNT; i++)
    if (AFFIXES[i].stat == stat)
      return i;
  return -1;
}

} // namespace

const UniqueDef UNIQUES[] = {
    {"Ancestral Blade", "Hurled_Axe", nullptr, "Every ancestor who held it still swings with you",
     18, {{S::Damage, 40}, {S::Crit, 10}, {S::Leech, 3}}},
    {"Ember Locket", "Amber_Charm", "Ember Locket", "Rise once from death in embers", 24,
     {{S::Burn, 15}, {S::Life, 20}}},
    {"Map of Old Roads", "Glass_Charm", "Map of Old Roads", "The minimap shows nearby landmarks",
     16, {{S::Speed, 8}, {S::Light, 30}}},
    {"Hearthstone", "Stone_Charm", "Hearthstone",
     "Below 25% health, return to your last lit shrine", 10, {{S::Regen, 2}, {S::Life, 15}}},
    {"Warden's Oath", "Plate", nullptr, "Nothing passes", 20,
     {{S::Armour, 4}, {S::Life, 40}, {S::Thorns, 15}}},
    {"Shrine Key", "Bone_Charm", "Shrine Key", "The first shrine each night kindles 50% faster",
     12, {{S::Kindling, 25}, {S::MagicFind, 20}}},
    {"Scholar's Lens", "Tome", nullptr, "Every page remembered", 8,
     {{S::Wisdom, 10}, {S::Haste, 8}}},
    {"Pack Banner", "Shield", "Pack Banner", "A spirit wolf fights at your side", 14,
     {{S::Life, 20}, {S::Armour, 2}}},
    {"Sea Charm", "Glass_Charm", "Sea Charm", "Standing in water heals 2/s", 6,
     {{S::ResistChill, 30}, {S::Speed, 6}}},
    {"Bone Dice", "Bone_Charm", "Bone Dice", "Champions drop an extra handful of loot", 22,
     {{S::MagicFind, 40}, {S::GoldFind, 40}}},
    {"Last Light", "Amber_Charm", "Last Light",
     "Once a night, a killing blow leaves you at 1 health", 28, {{S::Life, 25}, {S::Regen, 1}}},
    {"Crown of Dawn", "Robe", nullptr, "The dark remembers who ended it", 30,
     {{S::BossDamage, 10}, {S::EliteDamage, 10}, {S::Damage, 10}}},
};
const int UNIQUE_COUNT = int(std::size(UNIQUES));

bool makeUnique(Item &item, int ilvl, Rng &rng, int base) {
  std::vector<int> fit;
  for (int i = 0; i < UNIQUE_COUNT; i++)
    if (UNIQUES[i].qlvl <= ilvl && findBase(UNIQUES[i].base) >= 0 &&
        (base < 0 || findBase(UNIQUES[i].base) == base))
      fit.push_back(i);
  if (fit.empty())
    return false;
  forgeUnique(item, fit[size_t(rng.irange(0, int(fit.size()) - 1))]);
  return true;
}

void forgeUnique(Item &item, int k) {
  const UniqueDef &u = UNIQUES[k];
  item.base = findBase(u.base);
  item.rarity = Rarity::Unique;
  item.seed = uint32_t(k);
  item.curse = -1, item.count = 0;
  for (const auto &s : u.stats)
    if (s.stat != Stat::None)
      if (int a = affixFor(s.stat); a >= 0)
        item.rolls[item.count++] = Roll{a, 0, s.value};
}

const UniqueDef *uniqueOf(const Item &item) {
  if (item.rarity != Rarity::Unique || item.seed >= uint32_t(UNIQUE_COUNT))
    return nullptr;
  return &UNIQUES[item.seed];
}
