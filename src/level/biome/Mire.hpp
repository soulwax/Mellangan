#pragma once

#include "level/biome/Biome.hpp"

class Mire : public Biome {
public:
    Mire();
    Sprite dirtRim(int offset) const override;
    void weather(Level &level, double x, double y) const override;
};
