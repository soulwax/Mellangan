#include "particle/TextParticle.hpp"

#include "level/Level.hpp"
#include "particle/Particle.hpp"

void TextParticle::pop(Level &level, double x, double y, int value, Color c) {
  if (alive < 48)
    level.add<TextParticle>(x + rng.range(-3, 3), y - 4, value, c);
}

void TextParticle::tick() {
  Particle::tick();
  y -= 18 * DT * frac();
}

void TextParticle::render(Screen &s) {
  s.number(value, xp, yp, color.alpha(std::min(1.0f, frac() * 3)));
}
