#include "particle/debris/BloodDebris.hpp"

#include "particle/debris/Debris.hpp"

BloodDebris::BloodDebris(double x, double y, double z, Color c) : Debris(x, y, z) {
    drag = 0.96, bounce = 0.1, icon = 1, color = c;
}

void BloodDebris::render(Screen &s) { s.rect(xp, yp, 1, 1, color * 0.75f); }

void BloodDebris::renderShadow(Screen &s) { s.rect(xp, yp, 1, 1, {0, 0, 0, 0.35f}); }
