#include "entity/enemy/ai/Mend.hpp"

#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"

Enemy *hurtAlly(Enemy &e, float range) {
  Enemy *nearby[24], *best = nullptr;
  float worst = 0.9f;
  for (int i = 0, n = e.level->gather(e.x, e.y, range, nearby, 24); i < n; i++) {
    Enemy *o = nearby[i];
    float share = o->hp / std::max(1.0f, o->maxHp);
    if (o != &e && !o->removed && o->hp > 0 && share < worst)
      worst = share, best = o;
  }
  return best;
}

GoalState Mend::tick(Enemy &e, const Sense &, double &xs, double &ys) const {
  Enemy *ally = e.goalTime < seconds ? hurtAlly(e, range) : nullptr;
  if (!ally)
    return GoalState::Done;
  double xd = ally->x - e.x, yd = ally->y - e.y, d = std::sqrt(xd * xd + yd * yd);
  double v = d > 40 ? e.type.speed : 0;
  xs = d > 1e-3 ? xd / d * v : 0, ys = d > 1e-3 ? yd / d * v : 0;
  return GoalState::Running;
}
