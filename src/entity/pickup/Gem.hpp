#pragma once

#include "entity/pickup/Pickup.hpp"

class Gem : public Pickup {
public:
    float value;

    Gem(double x, double y, float value) : Pickup(x, y), value(value) {
        float dir = rng.range(0, TAU), v = rng.range(10, 30);
        xa = std::cos(dir) * v, ya = std::sin(dir) * v;
    }
    static void drop(Level &level, double x, double y, float value);

    void onAdded() override;
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

protected:
    void take(Player &p) override;

private:
    int tier() const { return value < 3 ? 0 : value < 12 ? 1 : 2; }
    Color color() const;
};
