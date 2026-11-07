#pragma once

#include <vector>

class Game;

struct Feat {
  const char *id, *title, *hint;
  bool (*met)(const Game &g, bool dawn);
};

extern const Feat FEATS[];
extern const int FEAT_COUNT;

std::vector<int> unlockFeats(Game &g, bool dawn, bool campaignOnly = false);
