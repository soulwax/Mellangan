#pragma once

#include "particle/debris/Debris.hpp"

class SmokeDebris : public Debris {
public:
    int lifeTime;

    SmokeDebris(double x, double y, double z);
    void render(Screen &s) override;
    void renderShadow(Screen &) override {}
};
