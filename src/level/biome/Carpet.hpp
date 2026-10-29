#pragma once

#include "level/biome/Biome.hpp"

class Carpet : public Biome {
public:
  Carpet(const BiomeData &data, BiomeId self, int set)
      : Biome(data), self(self), set(set) {}
  bool ground(int x, int y, Sprite *out) const override;
  Sprite transitionGround(int kind) const override {
    return atlas::carpet(set, kind);
  }

protected:
  BiomeId self;
  int set;

  bool covered(int x, int y) const;
  static Sprite edge(int set, int mask);
};

bool blockOf(bool (*test)(int, int, BiomeId), BiomeId id, int x, int y);
