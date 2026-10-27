#pragma once

#include "SpriteObject.hpp"

class Explosion : public SpriteObject {
public:
    int life, maxLife;

    Explosion(double x, double y, double z, int life = 5);
    int layer() const override { return 2; }
    void tick() override;
};
