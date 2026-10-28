#include "entity/enemy/ai/Approach.hpp"

#include "entity/enemy/Enemy.hpp"

GoalState Approach::tick(Enemy &e, const Sense &s, double &xs,
                         double &ys) const {
  e.aware = true;
  if (stop > 0 && s.dist < stop && e.cooldown <= 0)
    return GoalState::Done;
  double wave = std::sin(e.animT * gait.pulseRate);
  double v =
      e.type.speed *
      (gait.base + gait.pulse * (gait.rectified ? std::max(0.0, wave) : wave));
  double w = std::sin(e.animT * gait.weaveRate) * gait.weave;
  double spread = (int(e.seed >> 3 & 7) - 3.5) * 0.14 *
                  std::clamp((s.dist - 50) / 160, 0.0, 1.0);
  double xn = s.xn * std::cos(spread) - s.yn * std::sin(spread),
         yn = s.yn * std::cos(spread) + s.xn * std::sin(spread);
  xs = xn * v - yn * w, ys = yn * v + xn * w;
  return GoalState::Running;
}
