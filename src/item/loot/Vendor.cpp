#include "item/loot/Vendor.hpp"

#include <cstdio>

#include "entity/Player.hpp"
#include "item/loot/Loot.hpp"
#include "item/loot/Unique.hpp"

namespace {

struct Shelf {
  WareKind kind;
  float odds;
};

constexpr Shelf SHELVES[] = {
    {WareKind::Draught, 1}, {WareKind::Draught, 1}, {WareKind::Base, 1},
    {WareKind::Base, 1},    {WareKind::Base, 1},    {WareKind::Magic, 1},
    {WareKind::Magic, 1},   {WareKind::Magic, 0.5f}, {WareKind::Gamble, 1},
    {WareKind::Gamble, 1},  {WareKind::Gamble, 1},  {WareKind::Gamble, 1},
};

Item plain(int ilvl, Rng &rng) {
  Item item;
  item.ilvl = ilvl;
  item.base = pickBase(ilvl, item.tier, rng);
  item.seed = rng.next() | 1;
  return item;
}

Rng own(const Item &item) {
  Rng r;
  r.state = item.seed * 0x9E3779B97F4A7C15ull;
  return r;
}

} // namespace

const char *wareName(WareKind k) {
  constexpr const char *names[] = {"draught", "base", "magic", "gamble"};
  return names[int(k)];
}

Item gamble(const Item &base, Rng &rng) {
  Item item = base;
  item.seed = rng.next() | 1;
  item.curse = -1, item.count = 0;
  Rng r = own(item);
  RarityOdds o = rarityOdds(item.ilvl, 0);
  Rarity rarity = r.unit() < o.unique * 3 ? Rarity::Unique
                  : r.unit() < o.rare * 2 ? Rarity::Rare
                                          : Rarity::Magic;
  if (rarity == Rarity::Unique && makeUnique(item, item.ilvl, r, item.base))
    return item;
  affixItem(item, rarity, false, r);
  return item;
}

void Vendor::restock(int level, Rng &rng) {
  ilvl = std::max(1, level);
  restocks++;
  wares.clear();
  for (const Shelf &s : SHELVES) {
    if (!rng.chance(s.odds))
      continue;
    Ware w{s.kind, plain(ilvl, rng)};
    if (s.kind == WareKind::Magic) {
      Rng r = own(w.item);
      affixItem(w.item, Rarity::Magic, false, r);
    }
    wares.push_back(w);
  }
}

int Vendor::price(const Ware &w) const {
  float cut = 1 - discount;
  if (w.kind == WareKind::Draught)
    return std::max(1, int(MARKUP * (6 + 2 * ilvl) * cut));
  if (w.kind == WareKind::Gamble)
    return std::max(1, int(MARKUP * GAMBLE * w.item.value() * cut));
  return std::max(1, int(markup() * w.item.value() * cut));
}

bool Vendor::buy(Player &p, size_t i, Rng &rng) {
  if (i >= wares.size())
    return false;
  Ware &w = wares[i];
  int cost = price(w);
  bool bagged = w.kind != WareKind::Draught;
  if (w.sold || p.gold < cost || (bagged && int(p.bag.size()) >= Player::BAG))
    return false;
  p.gold -= cost;
  if (w.kind == WareKind::Draught)
    p.heal(DRAUGHT);
  else
    p.bag.push_back(w.kind == WareKind::Gamble ? gamble(w.item, rng) : w.item);
  w.sold = w.kind != WareKind::Gamble;
  return true;
}

bool Vendor::sell(Player &p, size_t bagIndex) {
  if (bagIndex >= p.bag.size())
    return false;
  p.gold += sellPrice(p.bag[bagIndex]);
  p.bag.erase(p.bag.begin() + long(bagIndex));
  return true;
}

int vendorSim(int minutes, uint64_t seed, bool print) {
  constexpr int VISIT = 3;
  Rng rng;
  rng.state = 0xC0FFEEull + seed * 7919;
  Vendor v;
  int gold = 0, dropped = 0, sales = 0, buys = 0, spent = 0, carried = 0, bag = 0;
  auto loot = [&](Source s, int mlvl) {
    for (Drop &d : rollLoot({s, std::min(99, mlvl)}, rng))
      if (d.kind == DropKind::Gold)
        gold += d.amount, dropped += d.amount;
      else if (d.kind == DropKind::Item && carried < Player::BAG)
        bag += Vendor::sellPrice(d.item), carried++;
  };
  for (int m = 0; m < minutes; m++) {
    int mlvl = 1 + 4 * m, kills = 30 + 10 * m;
    for (int k = 0; k < kills; k++)
      loot(k % 10 < 5 ? Source::Swarm : k % 10 < 9 ? Source::Line : Source::Brute, mlvl);
    loot(Source::Elite, mlvl + 2), loot(Source::Elite, mlvl + 2);
    if (m % 3 == 2)
      loot(Source::Champion, mlvl + 3);
    if (m % 10 == 9)
      loot(Source::Boss, mlvl + 5);
    if ((m + 1) % VISIT)
      continue;
    gold += bag, sales += bag, bag = carried = 0;
    v.restock(std::min(99, mlvl), rng);
    for (const Ware &w : v.wares)
      if (w.kind == WareKind::Magic && gold >= v.price(w))
        gold -= v.price(w), spent += v.price(w), buys++;
    if (print)
      std::printf("  minute %2d  ilvl %2d  gold %6d  magic buys %d\n", m + 1, v.ilvl, gold,
                  buys);
  }
  if (print)
    std::printf("%d minutes: %d gold dropped, %d from selling a full bag each visit, %d spent on %d "
                "magic buys\n",
                minutes, dropped, sales, spent, buys);
  return buys;
}
