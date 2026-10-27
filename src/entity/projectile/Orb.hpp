#pragma once

#include "entity/Entity.hpp"

class Orb : public Entity {
public:
    Orb(double x, double y, double xa, double ya, float damage) : damage(damage) {
        this->x = x, this->y = y;
        this->xa = xa, this->ya = ya;
    }
    int layer() const override { return 2; }
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

private:
    float damage, life = 5;
};
