#include "entity/enemy/ai/Flank.hpp"

#include <algorithm>
#include <cmath>

#include "entity/enemy/Enemy.hpp"

void Flank::start(Enemy &e, const Sense &) const {
  if (e.side == 0)
    e.side = (e.seed & 2) ? 1.0 : -1.0;
}

GoalState Flank::tick(Enemy &e, const Sense &s, double &xs, double &ys) const {
  e.aware = true;
  double spd = e.type.speed * (e.frenzy > 0 ? 1.35 : 1.0);
  double radial = std::clamp((s.dist - prefer) * 1.6, -spd, spd);
  double tangent = e.side * spd * 0.95;
  xs = s.xn * radial - s.yn * tangent;
  ys = s.yn * radial + s.xn * tangent;

  if (e.goalTime > 1.6f + (e.seed % 5) * 0.2f && s.dist < prefer + 30)
    return GoalState::Done;
  return GoalState::Running;
}
