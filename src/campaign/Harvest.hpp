#pragma once

#include <string>

#include "item/craft/Recipes.hpp"
#include "level/Edits.hpp"

class Level;
class Player;
struct Campaign;

struct Yield {
  const char *needles[24];
  Mat mat;
  int hits, amount, days;
};

struct PieceDef {
  Piece piece;
  const char *name;
  int cost[MATS];
};

constexpr int BUILDABLE = 5;
extern const PieceDef PIECES[BUILDABLE];

namespace harvest {

constexpr float REACH = 22, TOWN = 110, CAMPFIRE_SAFE = 80;

struct Target {
  int tx = 0, ty = 0;
  const Yield *yield = nullptr;
  bool placed = false;
  double x = 0, y = 0;
};

const Yield *yieldOf(const char *kind);
bool target(const Level &level, const Campaign &c, Target &out);
bool strike(Level &level, Campaign &c);
void aim(const Player &p, int &tx, int &ty);
std::string blocked(const Level &level, const Campaign &c, int piece, int tx, int ty);
bool build(Level &level, Campaign &c, int piece, int tx, int ty);
bool benchNear(const Level &level, const Campaign &c);
bool affordable(const Player &p, int piece);
int relight(Piece piece);

} // namespace harvest
