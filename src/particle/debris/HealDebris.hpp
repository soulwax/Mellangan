#pragma once

#include "particle/debris/Debris.hpp"

class HealDebris : public Debris {
public:
    bool lit;

    HealDebris(double x, double y, double z, int kind);
    void renderShadow(Screen &) override {}
    void renderLight(Screen &s) override;
};
