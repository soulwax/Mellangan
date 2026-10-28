#include "entity/enemy/AlphaWolf.hpp"

#include "audio/Sound.hpp"
#include "entity/enemy/ai/Falter.hpp"
#include "level/Level.hpp"

namespace {
const Falter SCATTER{1.0f};
} // namespace

void AlphaWolf::renderLight(Screen &s) {
  Enemy::renderLight(s);
  s.glow(xp, yp - 7 * scale, 36 * scale, {0.25f, 0.05f, 0.05f, 1});
}

void AlphaWolf::die() {
  Enemy::die();
  Sound::horde.play(0.4f, 0.3f);
  // scatter when alpha falls
  level->enemiesNear(x, y, 320, [](Enemy &o) {
    if (o.is(EnemyKind::Wolf)) {
      o.faltered = true;
      o.frenzy = 0;
      o.goals.clear();
      o.goals.push(&SCATTER);
    }
  });
}
