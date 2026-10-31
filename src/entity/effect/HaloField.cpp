#include "entity/effect/HaloField.hpp"

#include "entity/Player.hpp"
#include "item/weapon/Halo.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

void HaloField::tick() {
  if (!level->mob(ownerId))
    return remove();
  x = owner.x, y = owner.y;
  spin += DT * 0.9f;
  if (rng.chance(0.8f) && Spark::alive < 500) {
    float angle = rng.range(0, TAU);
    float r = halo.range() * rng.range(0.85f, 1.0f);
    level->add<Spark>(x + std::cos(angle) * r, y + std::sin(angle) * r - 4,
                      rng.range(-5, 5), -rng.range(4, 12),
                      Color{0.7f, 0.55f, 1, 1}, rng.range(0.4f, 0.8f),
                      rng.range(0.35f, 0.6f), false);
  }
}

void HaloField::render(Screen &s) {
  constexpr Color LILAC{0.78f, 0.62f, 1, 1};
  float r = halo.range(), flare = halo.pulse;
  double cy = owner.bodyY();
  s.ring(xp, cy, r, LILAC.alpha(0.2f + 0.45f * flare));
  for (int i = 0; i < 6; i++) {
    float a = spin + TAU * i / 6;
    s.particle(atlas::pt::Rune, xp + std::cos(a) * r, cy + std::sin(a) * r,
               LILAC.alpha(0.45f + 0.5f * flare), 1, a + PI * 0.5f);
  }
}

void HaloField::renderLight(Screen &s) {
  s.glow(xp, yp, halo.range() * 2.8f,
         Color{0.10f, 0.05f, 0.20f, 1} * (1 + halo.pulse * 0.6f));
}
