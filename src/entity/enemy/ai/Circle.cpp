#include "entity/enemy/ai/Circle.hpp"

#include "entity/enemy/Enemy.hpp"

GoalState Circle::tick(Enemy &e, const Sense &s, double &xs, double &ys) const {
  e.aware = true;
  if (e.goalTime >= seconds)
    return GoalState::Done;
  double v = e.type.speed, radial = std::clamp((s.dist - radius) * 2.0, -v, v);
  xs = s.xn * radial - s.yn * v * e.side, ys = s.yn * radial + s.xn * v * e.side;
  return GoalState::Running;
}
