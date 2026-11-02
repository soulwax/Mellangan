#pragma once

#include <string>

class Player;

struct DevStart {
  static constexpr const char *HELP =
      "F5 level up, F6 +30s, F7 god mode, F8 world debug, F9 warp to nearest "
      "structure, F10 next weather; --arm \"A,B!\" starts runs with those upgrades, "
      "--biome N starts in biome N, --at X,Y on tile X,Y, --lake at the nearest "
      "lakeshore, --cliff below the nearest cliff, --ramp below the nearest stairs, "
      "--peak on the nearest hilltop, --landmark N by landmark N, --give N puts N random "
      "items in the bag, --allies N summons N allies, --foes A,B rings the start with those "
      "enemy kinds, --loot drops loot";

  std::string arm, foes;
  int hero = -1, biome = -1, landmark = -1, x = 0, y = 0, give = 0, allies = 0;
  bool at = false, lake = false, cliff = false, ramp = false, peak = false;

  void parse(int argc, char **argv);
  void apply(Player &p);
  void armPlayer(Player &p) const;
  bool visitBiome(Player &p) const;
  bool stripsLevels() const { return !arm.empty() && arm.front() == '='; }

private:
  template <class F> void seek(F fits);
  void seekLandmark();
};
