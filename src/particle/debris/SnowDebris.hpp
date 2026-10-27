#pragma once

#include "particle/debris/Debris.hpp"

class SnowDebris : public Debris {
public:
    SnowDebris(double x, double y, double z, double wind);
    void render(Screen &s) override;
    void renderShadow(Screen &) override {}

protected:
    void collide(double xxa, double yya, double zza) override;
};
