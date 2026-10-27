#pragma once

#include "particle/debris/Debris.hpp"

class FlameDebris : public Debris {
public:
    int maxLife;
    bool noSmoke = false, lit;

    FlameDebris(double x, double y, double z);
    static void lick(Level &level, double x, double y, double z, float heat);
    void tick() override;
    void render(Screen &s) override;
    void renderShadow(Screen &) override {}
    void renderLight(Screen &s) override;
};
