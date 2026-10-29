#include "entity/effect/HaloField.hpp"

#include "entity/Player.hpp"
#include "item/weapon/Halo.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

void HaloField::tick() {
  x = player.x, y = player.y;
  if (rng.chance(0.8f) && Spark::alive < 500) {
    float angle = rng.range(0, TAU);
    float r = halo.range() * rng.range(0.85f, 1.0f);
    level->add<Spark>(x + std::cos(angle) * r, y + std::sin(angle) * r - 4,
                      rng.range(-5, 5), -rng.range(4, 12),
                      Color{0.7f, 0.55f, 1, 1}, rng.range(0.4f, 0.8f),
                      rng.range(0.35f, 0.6f), false);
  }
}

void HaloField::render(Screen &) {}

void HaloField::renderLight(Screen &s) {
  s.glow(xp, yp, halo.range() * 2.8f,
         Color{0.10f, 0.05f, 0.20f, 1} * (1 + halo.pulse * 0.6f));
}
