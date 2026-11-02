#pragma once

#include <string>
#include <vector>

#include "item/loot/Item.hpp"

class Player;

enum class Mat { Shard, Ore, Rune, Herb, Hide, Bone, Count };
constexpr int MATS = int(Mat::Count);

struct MatDef {
  const char *name, *plural;
  Color color;
  Sprite icon;
};

extern const MatDef MATERIALS[MATS];

struct Formula {
  const char *name, *text;
  int cost[MATS];
  int inputs;
  bool (*fits)(const Item &, const Formula &);
  Item (*make)(const std::vector<Item> &in, const Formula &, Rng &);
  int slot = -1;
  const char *fixed[2]{};
};

extern const Formula FORMULAS[];
extern const int FORMULA_COUNT;

std::string missing(const Player &p, const Formula &r, const std::vector<size_t> &picks);
int craft(Player &p, const Formula &r, std::vector<size_t> picks, Rng &rng);
