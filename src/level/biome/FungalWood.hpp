#pragma once

#include "level/biome/Carpet.hpp"

class FungalWood : public Carpet {
public:
  FungalWood();
  void weather(Level &level, double x, double y) const override;
};
