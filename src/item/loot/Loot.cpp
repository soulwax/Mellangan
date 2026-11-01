#include "item/loot/Loot.hpp"

#include <spdlog/fmt/fmt.h>

#include <cstdio>

namespace {

struct TreasureClass {
  int picks, none, gold, potion, material, item;
  Rarity floor;
  int floored;
};

const TreasureClass TREASURE[int(Source::Count)] = {
    {1, 92, 4, 2, 1, 1, Rarity::Normal, 0},
    {1, 80, 9, 5, 3, 3, Rarity::Normal, 0},
    {2, 60, 15, 8, 7, 10, Rarity::Normal, 0},
    {2, 30, 25, 10, 10, 25, Rarity::Normal, 0},
    {2, 0, 30, 10, 10, 50, Rarity::Magic, 1},
    {4, 0, 15, 10, 10, 65, Rarity::Rare, 2},
    {3, 10, 30, 20, 15, 25, Rarity::Normal, 0},
    {3, 0, 40, 0, 0, 60, Rarity::Magic, 1},
};

constexpr int NIGHT_TOUCHED = 25;

float diminish(float mf, float k) { return mf * k / (mf + k); }

int pickBase(int ilvl, int &tier, Rng &rng) {
  const auto &bases = itemBases();
  int total = 0;
  auto weight = [&](const ItemBase &b, int t) {
    int q = b.qlvl + TIER_LEVEL[t];
    return q > ilvl ? 0 : b.weight * (ilvl - q <= 10 ? 3 : 1);
  };
  for (const ItemBase &b : bases)
    for (int t = 0; t < TIER_COUNT; t++)
      total += weight(b, t);
  int r = rng.irange(0, std::max(0, total - 1));
  for (size_t i = 0; i < bases.size(); i++)
    for (int t = 0; t < TIER_COUNT; t++)
      if ((r -= weight(bases[i], t)) < 0)
        return tier = t, int(i);
  return tier = 0, 0;
}

Rarity rollRarity(int ilvl, float magicFind, Rng &rng) {
  RarityOdds o = rarityOdds(ilvl, magicFind);
  if (rng.unit() < o.unique)
    return Rarity::Unique;
  if (rng.unit() < o.rare)
    return Rarity::Rare;
  if (rng.unit() < o.magic)
    return Rarity::Magic;
  return rng.irange(0, 7) == 0 ? Rarity::Superior : Rarity::Normal;
}

void affixMagic(Item &item, Rng &rng) {
  int r = rng.irange(0, 3);
  bool pre = r != 2, suf = r >= 2;
  if (pre && !item.addAffix(true, rng))
    suf = true;
  if (suf && !item.addAffix(false, rng) && !item.count)
    item.addAffix(true, rng);
}

void affixRare(Item &item, int extra, Rng &rng) {
  int r = rng.irange(0, 9), want = (r < 4 ? 3 : r < 8 ? 4 : 5) + extra;
  for (int tries = 0; item.count < std::min(want, MAX_AFFIXES) && tries < 24; tries++) {
    bool pre = rng.chance(0.5f);
    if ((pre && item.prefixes() >= 3) || (!pre && item.suffixes() >= 3))
      pre = !pre;
    if ((pre && item.prefixes() >= 3) || (!pre && item.suffixes() >= 3))
      break;
    item.addAffix(pre, rng);
  }
}

void touch(Item &item, Rng &rng) {
  if (!item.count)
    return;
  Roll &r = item.rolls[rng.irange(0, item.count - 1)];
  r.tier = std::min(AFFIXES[r.affix].tiers - 1, r.tier + 2);
  item.reroll(int(&r - item.rolls), rng);
  item.curse = rng.irange(0, CURSE_COUNT - 1);
}

} // namespace

RarityOdds rarityOdds(int ilvl, float magicFind) {
  float scale = 1 + ilvl / 40.0f;
  return {std::min(1.0f, scale / 700 * (1 + diminish(magicFind, 250) / 100)),
          std::min(1.0f, scale / 90 * (1 + diminish(magicFind, 600) / 100)),
          std::min(1.0f, scale / 7 * (1 + magicFind / 100))};
}

Item rollItem(int ilvl, float magicFind, bool night, Rng &rng, Rarity floor) {
  Item item;
  item.ilvl = std::max(1, ilvl);
  item.base = pickBase(item.ilvl, item.tier, rng);
  item.seed = rng.next() | 1;
  Rng own;
  own.state = item.seed * 0x9E3779B97F4A7C15ull;
  Rarity r = rollRarity(item.ilvl, magicFind, own);
  if (int(r) < int(floor))
    r = floor;
  int extra = 0;
  if (r == Rarity::Unique)
    r = Rarity::Rare, extra = 1;
  item.rarity = r;
  if (r == Rarity::Magic)
    affixMagic(item, own);
  else if (r == Rarity::Rare)
    affixRare(item, extra, own);
  if (night && (r == Rarity::Magic || r == Rarity::Rare) &&
      own.irange(0, NIGHT_TOUCHED - 1) == 0)
    touch(item, own);
  return item;
}

std::vector<Drop> rollLoot(const LootRoll &roll, Rng &rng) {
  const TreasureClass &tc = TREASURE[int(roll.source)];
  std::vector<Drop> out;
  int itemWeight = tc.item * (roll.night ? 2 : 1);
  int total = tc.none + tc.gold + tc.potion + tc.material + itemWeight;
  for (int i = 0; i < tc.floored; i++)
    out.push_back({DropKind::Item, 1,
                   rollItem(roll.mlvl, roll.magicFind, roll.night, rng, tc.floor)});
  for (int i = 0; i < tc.picks; i++) {
    int r = rng.irange(0, total - 1);
    if ((r -= tc.none) < 0)
      continue;
    if ((r -= tc.gold) < 0) {
      int gold = int(roll.mlvl * rng.range(2, 6) * (1 + roll.goldFind / 100));
      out.push_back({DropKind::Gold, std::max(1, gold), {}});
    } else if ((r -= tc.potion) < 0) {
      out.push_back({DropKind::Potion, 1, {}});
    } else if ((r -= tc.material) < 0) {
      out.push_back({DropKind::Material, 1, {}});
    } else {
      out.push_back({DropKind::Item, 1,
                     rollItem(roll.mlvl, roll.magicFind, roll.night, rng)});
    }
  }
  return out;
}

int lootSim(int count, int mlvl) {
  Rng rng;
  rng.state = 0x5EEDull + uint64_t(mlvl) * 7919;
  int rarity[int(Rarity::Count)]{}, night = 0, affixes = 0, tiers[5]{};
  for (int i = 0; i < count; i++) {
    Item item = rollItem(mlvl, 0, true, rng);
    rarity[int(item.rarity)]++, night += item.night(), affixes += item.count;
    for (int k = 0; k < item.count; k++)
      tiers[item.rolls[k].tier]++;
  }
  RarityOdds o = rarityOdds(mlvl, 0);
  std::printf("%d items at item level %d (expected magic %.2f%%, rare %.2f%%, unique "
              "rolls %.3f%% become rares)\n",
              count, mlvl, o.magic * 100, o.rare * 100, o.unique * 100);
  for (int r = 0; r < int(Rarity::Count); r++)
    std::printf("  %-9s %6d  %6.2f%%\n", rarityName(Rarity(r)), rarity[r],
                100.0 * rarity[r] / count);
  std::printf("  night-touched %d, mean affixes %.2f, tiers I-V %d %d %d %d %d\n", night,
              double(affixes) / count, tiers[0], tiers[1], tiers[2], tiers[3], tiers[4]);
  Rng show;
  show.state = 42;
  for (int i = 0; i < 6; i++) {
    Item item = rollItem(mlvl, 300, true, show, Rarity::Magic);
    std::printf("  %s [%s]\n", item.name().c_str(), rarityName(item.rarity));
    for (int k = 0; k < item.count; k++)
      std::printf("    %s\n",
                  describe(AFFIXES[item.rolls[k].affix].stat, item.rolls[k].value).c_str());
    if (item.night())
      std::printf("    cursed: %s\n", CURSES[item.curse].text);
  }
  return 0;
}
