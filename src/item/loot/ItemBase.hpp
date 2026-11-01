#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "Atlas.hpp"
#include "item/loot/Affix.hpp"

struct ItemBase {
  std::string key;
  const char *name;
  unsigned slots;
  Sprite icon;
  int qlvl, weight, value;
  const char *grants = nullptr, *klass = nullptr;
  Stat implicit = Stat::None;
  float amount = 0;

  bool weapon() const { return slots & slot::Weapon; }
};

constexpr int TIERS = 3;
constexpr int TIER_LEVEL[TIERS] = {0, 18, 36};
extern const char *const TIER_NAMES[TIERS];

const std::vector<ItemBase> &itemBases();
int findBase(std::string_view key);
