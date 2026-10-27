#pragma once

#include "particle/debris/Debris.hpp"

class MeatDebris : public Debris {
public:
    MeatDebris(double x, double y, double z, Color c);
    void tick() override;

protected:
    void collide(double xxa, double yya, double zza) override;
};
