#pragma once

#include <entt/entity/entity.hpp>

#include <vector>

#include "item/loot/Item.hpp"

class Player;

enum class WareKind { Draught, Base, Magic, Gamble };

struct Ware {
  WareKind kind;
  Item item;
  bool sold = false;
};

struct Vendor {
  static constexpr float MARKUP = 4, GAMBLE = 6, DRAUGHT = 40;

  std::vector<Ware> wares;
  int ilvl = 1, camp = 0, restocks = 0;
  float discount = 0;
  bool day = false;
  entt::entity merchant = entt::null, bench = entt::null;

  void restock(int level, Rng &rng);
  float markup() const { return MARKUP + ilvl * ilvl / 300.0f; }
  int price(const Ware &w) const;
  bool buy(Player &p, size_t i, Rng &rng);
  static int sellPrice(const Item &item) { return std::max(1, item.value() / 4); }
  static bool sell(Player &p, size_t bagIndex);
};

Item gamble(const Item &base, Rng &rng);
const char *wareName(WareKind k);
int vendorSim(int minutes, uint64_t seed, bool print);
