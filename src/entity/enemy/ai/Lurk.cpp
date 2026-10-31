#include "entity/enemy/ai/Lurk.hpp"

#include "audio/Sound.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

GoalState Lurk::tick(Enemy &e, const Sense &s, double &xs, double &ys) const {
  if (!e.buried)
    return GoalState::Done;
  xs = ys = 0;
  if (s.dist >= wake)
    return GoalState::Running;
  e.buried = false, e.aware = true, e.frenzy = 1.5f;
  e.level->add<Shockwave>(e.x, e.bodyY(), 4, 30, 0.3f,
                          Color{1, 0.3f, 0.3f, 0.9f}, false);
  Spark::burst(*e.level, e.x, e.bodyY(), col::gold, 10, 90, true);
  e.voice(Sound::chest, 0.7f);
  e.voice(Sound::erupt, 0.5f);
  return GoalState::Done;
}
