#pragma once

#include <vector>

#include "item/loot/Item.hpp"

enum class Source { Swarm, Line, Brute, Elite, Champion, Boss, Chest, Mimic, Count };
enum class DropKind { Gold, Potion, Material, Item };

struct Drop {
  DropKind kind;
  int amount;
  Item item;
};

struct LootRoll {
  Source source;
  int mlvl;
  float magicFind = 0, goldFind = 0;
  bool night = false;
};

struct RarityOdds {
  float unique, rare, magic;
};

RarityOdds rarityOdds(int ilvl, float magicFind);
int pickBase(int ilvl, int &tier, Rng &rng);
void affixItem(Item &item, Rarity r, bool night, Rng &own);
Item rollItem(int ilvl, float magicFind, bool night, Rng &rng,
              Rarity floor = Rarity::Normal);
std::vector<Drop> rollLoot(const LootRoll &roll, Rng &rng);
int lootSim(int count, int mlvl);
