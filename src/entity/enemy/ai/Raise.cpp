#include "entity/enemy/ai/Raise.hpp"

#include <cmath>

#include "audio/Sound.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

GoalState Raise::tick(Enemy &e, const Sense &, double &xs, double &ys) const {
  if (e.cooldown > 0)
    return GoalState::Done;
  xs = ys = 0;
  if (e.goalTime < windup)
    return GoalState::Running;

  e.voice(Sound::raise, 0.3f, 0.1f);
  for (int i = 0; i < 2; i++) {
    double a = e.roll() * 6.2831853;
    double d = 20.0 + e.roll() * 22.0;
    double sx = e.x + std::cos(a) * d;
    double sy = e.y + std::sin(a) * d;
    e.level->spawner.spawn(EnemyKind::Skeleton, sx, sy, true);
    Spark::burst(*e.level, sx, sy - 6, {0.7f, 0.3f, 0.9f, 1}, 10, 50, false);
  }

  e.cooldown = cooldown + e.roll() * 2.0f;
  return GoalState::Done;
}

float Raise::tell(const Enemy &e) const {
  return e.cooldown <= 0 && e.goalTime < windup ? e.goalTime / windup : 0.0f;
}
