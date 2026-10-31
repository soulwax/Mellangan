#pragma once

#include "level/biome/Biome.hpp"

class Snow : public Biome {
public:
  Snow();
  bool winter() const override { return true; }
  Sprite grass(int kind) const override;
  Sprite dirtRim(int offset) const override;
  Sprite lakeRim(int offset) const override {
    return atlas::waterEdge(16, offset);
  }
  Sprite frost(Sprite ground) const override;
  void weather(Level &level, double x, double y) const override;
};
