#include "particle/debris/HealDebris.hpp"

#include "particle/debris/Debris.hpp"

HealDebris::HealDebris(double x, double y, double z, int kind)
    : Debris(x, y, z), lit(rng.chance(0.33f)) {
  life /= 2;
  drag = 0.92, gravity = 0, icon = 8 * 3 + kind;
}

void HealDebris::renderLight(Screen &s) {
  if (!lit)
    return;
  s.glow(xp, yp, 22,
         icon == 24 ? Color{0.30f, 0.06f, 0.06f, 1}
                    : Color{0.10f, 0.10f, 0.35f, 1});
}
