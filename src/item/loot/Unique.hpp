#pragma once

#include "item/loot/Affix.hpp"

struct Item;
struct Rng;

struct UniqueDef {
  const char *name, *base, *grants, *text;
  int qlvl;
  struct {
    Stat stat;
    float value;
  } stats[4];
};

extern const UniqueDef UNIQUES[];
extern const int UNIQUE_COUNT;

void forgeUnique(Item &item, int index);
bool makeUnique(Item &item, int ilvl, Rng &rng, int base = -1);
const UniqueDef *uniqueOf(const Item &item);
