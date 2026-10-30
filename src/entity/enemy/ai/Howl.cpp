#include "entity/enemy/ai/Howl.hpp"

#include "audio/Sound.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Shockwave.hpp"
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

  int pack = 0;
  e.level->enemiesNear(e.x, e.y, 240, [&](Enemy &o) {
    if (o.is(EnemyKind::Wolf))
      o.frenzy = 6.0f, pack++;
  });

  if (e.boss) {
    e.level->add<Shockwave>(e.x, e.bodyY(), 10, 120, 0.6f,
                            Color{0.95f, 0.2f, 0.25f, 0.9f})->spokes = 16;
    for (int i = 0; pack < 10 && i < 3; i++, pack++) {
      double a = e.roll() * TAU, d = 90 + e.roll() * 30;
      double wx = e.x + std::cos(a) * d, wy = e.y + std::sin(a) * d;
      e.level->spawner.spawn(EnemyKind::Wolf, wx, wy, true);
      Spark::burst(*e.level, wx, wy - 6, Color{0.9f, 0.2f, 0.25f, 1}, 8, 50,
                   false);
    }
  }

  return GoalState::Done;
}

float Howl::tell(const Enemy &e) const {
  return e.cooldown <= 0 && e.goalTime < windup ? e.goalTime / windup : 0.0f;
}
