#include "particle/debris/FlameDebris.hpp"

#include "level/Level.hpp"
#include "particle/debris/Debris.hpp"
#include "particle/debris/SmokeDebris.hpp"

FlameDebris::FlameDebris(double x, double y, double z)
    : Debris(x, y, z), lit(rng.chance(0.25f)) {
  maxLife = life /= 2;
  drag = 0.92, gravity = 0, icon = rng.irange(8, 10);
}

void FlameDebris::lick(Level &level, double x, double y, double z, float heat) {
  if (!room())
    return;
  auto f = std::make_unique<FlameDebris>(x, y, z);
  f->xa *= 0.12 * heat, f->ya *= 0.12 * heat,
      f->za = rng.range(0.25f, 0.6f) * heat;
  f->gravity = -0.015, f->drag = 0.9, f->lit = false;
  f->maxLife = f->life = rng.irange(8, 16);
  level.spawn(std::move(f));
}

void FlameDebris::tick() {
  Debris::tick();
  if (!removed || noSmoke || !rng.chance(0.2f))
    return;
  shed(std::make_unique<SmokeDebris>(x, y, z), 0.1, 1);
}

void FlameDebris::render(Screen &s) {
  icon = 8 * 2 + 3 - std::min(3, 4 * std::max(life, 0) / maxLife);
  Debris::render(s);
}

void FlameDebris::renderLight(Screen &s) {
  if (lit)
    s.glow(xp, yp, 26,
           Color{0.40f, 0.20f, 0.05f, 1} *
               (float(std::max(life, 0)) / maxLife));
}
