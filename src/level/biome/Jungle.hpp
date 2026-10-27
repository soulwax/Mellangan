#pragma once

#include "level/biome/Biome.hpp"

class Jungle : public Biome {
public:
    Jungle();
    Sprite dirtRim(int offset) const override;
};
