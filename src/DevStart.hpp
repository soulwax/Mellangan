#pragma once

#include <string>

class Player;

struct DevStart {
  std::string arm;
  int hero = -1, biome = -1, landmark = -1, x = 0, y = 0;
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
