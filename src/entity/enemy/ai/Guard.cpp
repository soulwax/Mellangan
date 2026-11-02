#include "entity/enemy/ai/Guard.hpp"

#include <cmath>

#include "entity/enemy/Enemy.hpp"

GoalState Guard::tick(Enemy &e, const Sense &s, double &xs, double &ys) const {
  e.aware = true;
  if (turn > 0) {
    double a = std::atan2(e.yMark, e.xMark), b = std::atan2(s.yn, s.xn);
    double d = std::remainder(b - a, TAU), step = turn * DT;
    a += std::clamp(d, -step, step);
    e.xMark = std::cos(a), e.yMark = std::sin(a);
  } else {
    e.xMark = s.xn, e.yMark = s.yn;
  }
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
