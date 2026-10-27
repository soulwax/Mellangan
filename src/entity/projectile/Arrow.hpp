#pragma once

#include "entity/projectile/Projectile.hpp"

class Arrow : public Projectile {
public:
    Arrow(double x, double y, double xa, double ya, float damage, int pierce)
        : Projectile(x, y, xa, ya, 1.2f, damage, 35, pierce) {
        radius = 5;
    }
    void tick() override;
    void render(Screen &s) override;
};
