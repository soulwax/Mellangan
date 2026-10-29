#include "entity/enemy/ai/Howl.hpp"

#include "audio/Sound.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

GoalState Howl::tick(Enemy &e, const Sense &, double &xs, double &ys) const {
  if (e.cooldown > 0)
    return GoalState::Done;
  xs = ys = 0;
  if (e.goalTime < windup)
    return GoalState::Running;

  e.voice(Sound::howl, 0.3f, 0.12f);
  Spark::burst(*e.level, e.x, e.bodyY(), col::gold, 16, 75, true);
  e.frenzy = 6.0f;
  e.cooldown = cooldown;

  e.level->enemiesNear(e.x, e.y, 240, [](Enemy &o) {
    if (o.is(EnemyKind::Wolf))
      o.frenzy = 6.0f;
  });

  return GoalState::Done;
}

float Howl::tell(const Enemy &e) const {
  return e.cooldown <= 0 && e.goalTime < windup ? e.goalTime / windup : 0.0f;
}
