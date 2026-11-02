#pragma once

#include <vector>

#include "item/loot/Item.hpp"

enum class Source { Swarm, Line, Brute, Elite, Champion, Boss, Chest, Mimic, Count };
enum class DropKind { Gold, Potion, Material, Item };
enum class Lean { None, Gold, Ore, Arcane, Charm, Hide, Riches };

struct Drop {
  DropKind kind;
  int amount;
  Item item;
  int material = 0;
};

struct LootRoll {
  Source source;
  int mlvl;
  float magicFind = 0, goldFind = 0;
  bool night = false;
  Lean lean = Lean::None;
};

struct RarityOdds {
  float unique, rare, magic;
};

RarityOdds rarityOdds(int ilvl, float magicFind);
int pickBase(int ilvl, int &tier, Rng &rng);
void affixItem(Item &item, Rarity r, bool night, Rng &own);
Item rollItem(int ilvl, float magicFind, bool night, Rng &rng,
              Rarity floor = Rarity::Normal, unsigned prefer = 0);
std::vector<Drop> rollLoot(const LootRoll &roll, Rng &rng);
int lootSim(int count, int mlvl);
