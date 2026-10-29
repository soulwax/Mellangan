#pragma once

#include <cstdint>

class Level;
class Player;

class EncounterObjective {
public:
  int kind;
  float progress = 0, integrity = 100;
  double heading, distance = 0;

  explicit EncounterObjective(uint32_t seed);
  bool tick(Level &level, Player &player, double &x, double &y, double distance2);
  bool complete() const;
  float fraction(int defeated, int defenders) const;
};
