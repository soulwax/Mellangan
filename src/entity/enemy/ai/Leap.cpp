#include "entity/enemy/ai/Leap.hpp"

#include <cmath>

#include "entity/enemy/Enemy.hpp"

void Leap::start(Enemy &e, const Sense &s) const {
  e.xMark = e.x + s.xd, e.yMark = e.y + s.yd;
}

GoalState Leap::tick(Enemy &e, const Sense &, double &xs, double &ys) const {
  e.aware = true;
  if (e.goalTime < crouch) {
    xs = ys = 0;
    return GoalState::Running;
  }
  float t = e.goalTime - crouch;
  if (t < flight) {
    double xd = e.xMark - e.x, yd = e.yMark - e.y, d = std::sqrt(xd * xd + yd * yd);
    double v = d > 1 ? std::min(320.0, d / std::max(DT, flight - t)) : 0;
    xs = d > 1 ? xd / d * v : 0, ys = d > 1 ? yd / d * v : 0;
    e.z = std::sin(t / flight * PI) * height;
    return GoalState::Running;
  }
  xs = ys = 0;
  e.z = 0, e.pounced = true, e.cooldown = cooldown;
  return GoalState::Done;
}

float Leap::tell(const Enemy &e) const {
  return e.goalTime < crouch ? e.goalTime / crouch : 0;
}
