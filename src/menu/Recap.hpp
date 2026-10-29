#pragma once

#include <string>

#include "gfx/Screen.hpp"

class Level;

struct RunStats {
  std::string rank;
  Color rankColor;
  std::string className;
  Color classColor;
  std::string difficulty;
  Color diffColor;
  std::string distance;
  std::string totalDamage;
  std::string timeText;
  bool isRecord = false;
  int kills = 0;
  int shrines = 0;
  int level = 1;
};

RunStats computeRunStats(const Level &l);
float renderEndStats(Screen &s, const Level &l, float y, float alpha);
float recap(Screen &s, const Level &l, float y, float alpha);
