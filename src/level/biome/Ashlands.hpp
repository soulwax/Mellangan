#pragma once

#include "level/biome/Carpet.hpp"

class Ashlands : public Carpet {
public:
  Ashlands();
  bool ground(int x, int y, Sprite *out) const override;
  bool fixedGround(int x, int y) const override;
  int place(int x, int y, uint32_t roll, Prop &p) const override;
  void weather(Level &level, double x, double y) const override;
};
