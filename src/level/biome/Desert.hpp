#pragma once

#include "level/biome/Biome.hpp"

class Desert : public Biome {
public:
  Desert();
  bool ground(int x, int y, Sprite *out) const override;
  bool fixedGround(int x, int y) const override;
  Sprite transitionGround(int kind) const override {
    return atlas::flora(kind == 0 ? atlas::fl::Sand
                                 : kind == 1 ? atlas::fl::SandSpeck
                                             : atlas::fl::SandPebbles);
  }
  int place(int x, int y, uint32_t roll, Prop &p) const override;
};
