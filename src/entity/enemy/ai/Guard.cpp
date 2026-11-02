#include "entity/enemy/ai/Guard.hpp"

#include "entity/enemy/Enemy.hpp"

GoalState Guard::tick(Enemy &e, const Sense &s, double &xs, double &ys) const {
  e.aware = true;
  e.xMark = s.xn, e.yMark = s.yn;
  if (s.dist < close || e.goalTime >= seconds)
    return GoalState::Done;
  xs = s.xn * e.type.speed * pace, ys = s.yn * e.type.speed * pace;
  return GoalState::Running;
}

GoalState Totem::tick(Enemy &e, const Sense &, double &xs, double &ys) const {
  e.aware = true;
  xs = ys = 0;
  return GoalState::Running;
}
