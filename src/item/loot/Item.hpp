#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "Math.hpp"
#include "gfx/Screen.hpp"
#include "item/loot/ItemBase.hpp"

enum class Rarity { Normal, Superior, Magic, Rare, Unique, Crafted, Count };

struct Roll {
  int affix, tier;
  float value;
};

struct Curse {
  const char *name, *text;
};

constexpr int MAX_AFFIXES = 6;
constexpr int CURSE_COUNT = 5;
extern const Curse CURSES[CURSE_COUNT];

struct Item {
  int base = 0, tier = 0, ilvl = 1;
  Rarity rarity = Rarity::Normal;
  uint32_t seed = 1;
  int curse = -1, count = 0;
  Roll rolls[MAX_AFFIXES]{};

  const ItemBase &def() const { return itemBases()[base]; }
  bool night() const { return curse >= 0; }
  int prefixes() const;
  int suffixes() const;
  bool hasFamily(int affix) const;
  bool addAffix(bool prefix, Rng &rng);
  void reroll(int i, Rng &rng);
  std::string baseName() const;
  std::string name() const;
  Color color() const;
  int value() const;
  std::string save() const;
  static bool load(std::string_view line, Item &out);
};

const char *rarityName(Rarity r);
