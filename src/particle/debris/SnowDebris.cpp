#include "particle/debris/SnowDebris.hpp"

SnowDebris::SnowDebris(double x, double y, double z, double wind) : Debris(x, y, z) {
    life = rng.irange(150, 260);
    drag = 0.99, gravity = 0.004, bounce = 0;
    xa = xa * 0.12 + wind, ya *= 0.05, za = -rng.range(0.1f, 0.3f);
}

void SnowDebris::render(Screen &s) { s.rect(xp, yp, 1, 1, {0.92f, 0.95f, 1.0f, 0.85f}); }

void SnowDebris::collide(double, double, double zza) {
    if (zza != 0) remove();
}
