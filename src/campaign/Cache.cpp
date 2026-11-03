#include "campaign/Cache.hpp"

#include <cmath>

#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {

constexpr int RELIQUARY = 90;
constexpr Color GILT{1, 0.85f, 0.45f, 1}, SEAL{0.6f, 0.4f, 1, 1};

} // namespace

void Cache::onAdded() { level->tag(this); }

void Cache::tick() {
  t += DT;
  if (!sealed && level->ticks % 9 == 0)
    Spark::burst(*level, x + std::sin(t * 3) * 6, y - 10, GILT, 1, 20, true);
}

void Cache::render(Screen &s) {
  float bob = std::sin(t * 2) * 0.5f;
  s.sprite(atlas::tile(RELIQUARY), xp, yp + bob, 8, 16, 1, 1);
  if (!sealed)
    return;
  float k = 0.6f + 0.4f * std::sin(t * 4);
  s.ring(xp, yp - 6, 11, SEAL.alpha(0.5f * k));
  s.ring(xp, yp - 6, 14, SEAL.alpha(0.25f * k));
}

void Cache::renderShadow(Screen &s) {
  s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, 1, 0.6f);
}

void Cache::renderLight(Screen &s) {
  s.glow(xp, yp - 6, sealed ? 50 : 80, (sealed ? SEAL : GILT) * 0.3f);
  level->lights.add(x, y, 60, 0.5f);
}
