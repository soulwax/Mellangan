#include "item/craft/Recipes.hpp"

#include <algorithm>
#include <iterator>

#include <spdlog/fmt/fmt.h>

#include "entity/Player.hpp"
#include "item/loot/Loot.hpp"

namespace {

namespace kt = atlas::kt;
namespace fr = atlas::fr;

Rng seeded(Item &item, Rng &rng) {
  item.seed = rng.next() | 1;
  Rng own;
  own.state = item.seed * 0x9E3779B97F4A7C15ull;
  return own;
}

bool affixed(const Item &i, const Formula &) {
  return i.count > 0 && (i.rarity == Rarity::Magic || i.rarity == Rarity::Rare ||
                         i.rarity == Rarity::Crafted);
}

bool rare(const Item &i, const Formula &) { return i.rarity == Rarity::Rare && !i.night(); }

bool rough(const Item &i, const Formula &) { return i.tier < TIER_COUNT - 1; }

bool magic(const Item &i, const Formula &) { return i.rarity == Rarity::Magic; }

bool touched(const Item &i, const Formula &) { return i.night(); }

bool plain(const Item &i, const Formula &r) {
  return int(i.rarity) <= int(Rarity::Superior) && i.gearSlot() == r.slot;
}

Item reroll(const std::vector<Item> &in, const Formula &, Rng &rng) {
  Item out = in[0];
  int weakest = 0;
  for (int i = 1; i < out.count; i++)
    if (out.quality(i) < out.quality(weakest))
      weakest = i;
  out.reroll(weakest, rng);
  return out;
}

Item reforge(const std::vector<Item> &in, const Formula &, Rng &rng) {
  Item out = in[0];
  uint32_t name = out.seed;
  Rng own = seeded(out, rng);
  out.count = 0;
  affixItem(out, Rarity::Rare, false, own);
  out.seed = name;
  return out;
}

Item upgrade(const std::vector<Item> &in, const Formula &, Rng &) {
  Item out = in[0];
  out.tier++;
  return out;
}

Item combine(const std::vector<Item> &in, const Formula &, Rng &rng) {
  Item out;
  out.base = in[0].base, out.tier = in[0].tier;
  out.ilvl = std::min({in[0].ilvl, in[1].ilvl, in[2].ilvl});
  Rng own = seeded(out, rng);
  affixItem(out, Rarity::Rare, false, own);
  return out;
}

Item cleanse(const std::vector<Item> &in, const Formula &, Rng &rng) {
  Item out = in[0];
  out.curse = -1;
  int boosted = 0;
  for (int i = 1; i < out.count; i++)
    if (out.rolls[i].tier > out.rolls[boosted].tier)
      boosted = i;
  if (out.count) {
    out.rolls[boosted].tier = std::max(0, out.rolls[boosted].tier - 2);
    out.reroll(boosted, rng);
  }
  return out;
}

Item forge(const std::vector<Item> &in, const Formula &r, Rng &rng) {
  Item out = in[0];
  out.rarity = Rarity::Crafted, out.count = 0;
  Rng own = seeded(out, rng);
  for (const char *key : r.fixed)
    if (key)
      out.addFixed(findAffix(key), own);
  int want = out.count + own.irange(1, 3);
  for (int tries = 0; out.count < std::min(want, MAX_AFFIXES) && tries < 16; tries++) {
    bool pre = own.chance(0.5f);
    if ((pre ? out.prefixes() : out.suffixes()) >= 3)
      pre = !pre;
    if ((pre ? out.prefixes() : out.suffixes()) < 3)
      out.addAffix(pre, own);
  }
  return out;
}

int owned(const Player &p, Mat m) { return p.materials[int(m)]; }

} // namespace

const MatDef MATERIALS[MATS] = {
    {"crystal shard", "crystal shards", {0.75f, 0.6f, 1, 1}, atlas::kit(kt::Amethyst)},
    {"ore", "ore", {0.85f, 0.55f, 0.35f, 1}, atlas::frontier(fr::OrePile)},
    {"rune stone", "rune stones", {0.5f, 0.85f, 0.8f, 1}, atlas::frontier(fr::RuneStoneLow)},
    {"herb", "herbs", {0.55f, 0.85f, 0.4f, 1}, atlas::kit(kt::Sage)},
    {"hide", "hides", {0.8f, 0.6f, 0.4f, 1}, atlas::beast(atlas::bs::Hide)},
    {"bone", "bones", {0.92f, 0.88f, 0.76f, 1}, atlas::beast(atlas::bs::Bone)},
    {"wood", "wood", {0.72f, 0.5f, 0.3f, 1}, atlas::town(atlas::tt::Log)},
    {"stone", "stone", {0.7f, 0.7f, 0.72f, 1}, atlas::kit(kt::Pebbles)},
};

const Formula FORMULAS[] = {
    {"Reroll", "rerolls the weakest affix within its tier", {3, 0, 0, 0}, 1, affixed, reroll},
    {"Reforge", "rerolls every affix of a rare; the name stays", {5, 2, 0, 0}, 1, rare,
     reforge},
    {"Upgrade", "raises a Worn or Fine item one tier", {0, 6, 1, 0}, 1, rough, upgrade},
    {"Combine", "three magic items of one slot become one rare", {0, 0, 0, 0}, 3, magic,
     combine},
    {"Cleanse", "lifts a night curse; the night's gift fades", {1, 0, 0, 1}, 1, touched,
     cleanse},
    {"Bloodfang", "a weapon of damage and leech", {2, 2, 0, 0, 0, 2}, 1, plain, forge, GearWeapon,
     {"damage", "leech"}},
    {"Stormbrand", "a weapon of shock and haste", {3, 2, 1, 0}, 1, plain, forge, GearWeapon,
     {"shock", "haste"}},
    {"Ember Lamp", "an off-hand of burn and area", {3, 0, 1, 2}, 1, plain, forge, GearOffhand,
     {"burn", "area"}},
    {"Sage's Ward", "an off-hand of armour and wisdom", {0, 4, 1, 1}, 1, plain, forge,
     GearOffhand, {"armour", "wisdom"}},
    {"Frostmantle", "armour against the cold", {2, 2, 0, 0, 2, 0}, 1, plain, forge, GearArmour,
     {"armour", "resist chill"}},
    {"Thornhide", "armour of thorns and life", {0, 2, 0, 1, 3, 0}, 1, plain, forge, GearArmour,
     {"thorns", "life"}},
    {"Blood Charm", "a charm of life and leech", {1, 0, 1, 1, 0, 2}, 1, plain, forge, GearCharm,
     {"life", "leech"}},
    {"Fortune Charm", "a charm of magic and gold find", {4, 0, 2, 0}, 1, plain, forge,
     GearCharm, {"fortune", "gleaming"}},
};
const int FORMULA_COUNT = int(std::size(FORMULAS));

std::string missing(const Player &p, const Formula &r, const std::vector<size_t> &picks) {
  for (int m = 0; m < MATS; m++)
    if (owned(p, Mat(m)) < r.cost[m])
      return fmt::format("needs {} {}", r.cost[m],
                         r.cost[m] == 1 ? MATERIALS[m].name : MATERIALS[m].plural);
  if (int(picks.size()) < r.inputs)
    return r.inputs == 1 ? "pick an item from your bag"
                         : fmt::format("pick {} more items", r.inputs - int(picks.size()));
  for (size_t k : picks)
    if (k >= p.bag.size() || !r.fits(p.bag[k], r))
      return "that item does not fit this recipe";
  for (size_t i = 0; i < picks.size(); i++)
    for (size_t j = i + 1; j < picks.size(); j++)
      if (picks[i] == picks[j])
        return "pick different items";
  for (size_t k : picks)
    if (p.bag[k].gearSlot() != p.bag[picks[0]].gearSlot())
      return "the items must share a slot";
  if (int(picks.size()) > r.inputs)
    return "too many items picked";
  return {};
}

int craft(Player &p, const Formula &r, std::vector<size_t> picks, Rng &rng) {
  if (!missing(p, r, picks).empty())
    return -1;
  std::vector<Item> in;
  for (size_t k : picks)
    in.push_back(p.bag[k]);
  for (int m = 0; m < MATS; m++)
    p.materials[m] -= r.cost[m];
  Item made = r.make(in, r, rng);
  if (picks.size() == 1)
    return p.bag[picks[0]] = made, int(picks[0]);
  std::sort(picks.rbegin(), picks.rend());
  for (size_t k : picks)
    p.bag.erase(p.bag.begin() + std::ptrdiff_t(k));
  p.bag.push_back(made);
  return int(p.bag.size()) - 1;
}
