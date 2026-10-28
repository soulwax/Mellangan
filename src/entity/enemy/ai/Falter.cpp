#include "entity/enemy/ai/Falter.hpp"

#include "entity/enemy/Enemy.hpp"

bool Falter::urgent(const Enemy &e, const Sense &s) const {
  return !e.faltered && e.hp < e.maxHp * below && s.dist < 160;
}

void Falter::start(Enemy &e, const Sense &) const { e.faltered = true; }

GoalState Falter::tick(Enemy &e, const Sense &s, double &xs, double &ys) const {
  if (e.goalTime > 1.1f)
    return GoalState::Done;
  double v = e.type.speed * 0.9, w = std::sin(e.animT * 6) * 20 * e.side;
  xs = -s.xn * v - s.yn * w, ys = -s.yn * v + s.xn * w;
  return GoalState::Running;
}
