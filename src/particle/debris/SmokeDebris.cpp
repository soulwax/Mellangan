#include "particle/debris/SmokeDebris.hpp"

#include "particle/debris/Debris.hpp"

SmokeDebris::SmokeDebris(double x, double y, double z) : Debris(x, y, z) {
    lifeTime = life /= 2;
    drag = 0.92, gravity = -0.02, icon = rng.irange(8, 10);
}

void SmokeDebris::render(Screen &s) { s.particle(icon, xp, yp, {0.45f, 0.43f, 0.48f, 0.7f * float(life) / lifeTime}); }
