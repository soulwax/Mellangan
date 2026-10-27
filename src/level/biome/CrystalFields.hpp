#pragma once

#include "level/biome/Carpet.hpp"

class CrystalFields : public Carpet {
public:
  CrystalFields();
  void weather(Level &level, double x, double y) const override;
};
