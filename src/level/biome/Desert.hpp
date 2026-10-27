#pragma once

#include "level/biome/Biome.hpp"

class Desert : public Biome {
public:
    Desert();
    bool ground(int x, int y, Sprite *out) const override;
    int place(int x, int y, uint32_t roll, Prop &p) const override;
};
