#pragma once

#include "level/biome/Biome.hpp"

class ExpansionBiome : public Biome {
public:
  int index;
  explicit ExpansionBiome(int index);
  bool ground(int x, int y, Sprite *out) const override;
  Sprite transitionGround(int kind) const override {
    return atlas::biomeGround(index, kind);
  }
  Sprite dirtRim(int offset) const override {
    return atlas::dirtEdge(6 + index, offset);
  }
};

const Biome *expansionBiome(int index);
