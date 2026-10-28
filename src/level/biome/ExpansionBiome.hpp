#pragma once

#include "level/biome/Biome.hpp"

class ExpansionBiome : public Biome {
public:
  int index;
  explicit ExpansionBiome(int index);
  bool ground(int x, int y, Sprite *out) const override;
};

const Biome *expansionBiome(int index);
