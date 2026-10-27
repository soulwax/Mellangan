#include "entity/enemy/ai/LookOut.hpp"

#include "entity/enemy/Enemy.hpp"

void LookOut::start(Enemy &e, const Sense &) const {
  double a = e.roll() * TAU;
  e.xMark = std::cos(a), e.yMark = std::sin(a);
}

GoalState LookOut::tick(Enemy &e, const Sense &s, double &xs,
                        double &ys) const {
  if (s.dist < sight)
    e.aware = true;
  if (e.aware)
    return GoalState::Done;
  if (e.goalTime > 1.5f + e.roll())
    start(e, s), e.goalTime = 0;
  xs = e.xMark * e.type.speed * 0.3, ys = e.yMark * e.type.speed * 0.3;
  return GoalState::Running;
}
