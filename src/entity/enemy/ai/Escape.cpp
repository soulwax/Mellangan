#include "entity/enemy/ai/Escape.hpp"

#include <cmath>

#include "audio/Sound.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

GoalState Escape::tick(Enemy &e, const Sense &s, double &xs, double &ys) const {
  e.aware = true;
  e.age += DT;

  // run away from player
  double spd = e.type.speed;
  double weave = std::sin(e.animT * 9) * 22;
  xs = -s.xn * spd + s.yn * weave;
  ys = -s.yn * spd - s.xn * weave;

  // leve a spark trail
  if (e.level->ticks % 6 == 0)
    Spark::burst(*e.level, e.x, e.bodyY(), col::gold, 2, 20, false);

  if (e.age >= duration - 2.0f) {
    xs *= 0.35;
    ys *= 0.35;
  }

  if (e.age >= duration) {
    // Escaped into portal
    Sound::nova.play(0.45f, 0.4f);
    Spark::burst(*e.level, e.x, e.bodyY(), col::gold, 30, 120, true);
    e.remove();
    return GoalState::Done;
  }

  return GoalState::Running;
}

float Escape::tell(const Enemy &e) const {
  if (e.age >= duration - 2.0f)
    return (e.age - (duration - 2.0f)) / 2.0f;
  return 0.0f;
}
