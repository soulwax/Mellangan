#pragma once

#include "level/biome/Biome.hpp"

class Blossom : public Biome {
public:
  Blossom();
  Sprite grass(int kind) const override;
};
