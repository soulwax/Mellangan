#include "entity/enemy/ai/Ranged.hpp"

#include "entity/enemy/Enemy.hpp"

GoalState Ranged::tick(Enemy &e, const Sense &s, double &xs, double &ys) const {
  e.aware = true;
  if (s.dist > range + 80)
    return GoalState::Done;
  double v = e.type.speed, radial = std::clamp((s.dist - prefer) * 1.5, -v, v),
         w = std::cos(e.animT * 3) * 28;
  xs = s.xn * radial - s.yn * w, ys = s.yn * radial + s.xn * w;
  if (e.cooldown <= 0 && s.dist < range)
    e.wantsFire = true, e.cooldown = 2.5f + 1.5f * e.roll();
  return GoalState::Running;
}
