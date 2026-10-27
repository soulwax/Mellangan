#include "entity/enemy/ai/Stalk.hpp"

#include "entity/enemy/Enemy.hpp"

GoalState Stalk::tick(Enemy &e, const Sense &s, double &xs, double &ys) const {
  e.aware = true;
  if ((!s.seen && s.dist < range * 1.6) || e.goalTime > 7)
    return GoalState::Done;
  double v = e.type.speed, radial = std::clamp((s.dist - range) * 1.5, -v, v),
         side = e.side * v * 0.7;
  xs = s.xn * radial - s.yn * side, ys = s.yn * radial + s.xn * side;
  return GoalState::Running;
}
