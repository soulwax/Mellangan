#include "entity/enemy/ai/Hide.hpp"

#include "entity/enemy/Enemy.hpp"

bool Hide::urgent(const Enemy &e, const Sense &s) const { return e.aware && s.seen && s.dist < range; }

GoalState Hide::tick(Enemy &e, const Sense &s, double &xs, double &ys) const {
    if (!s.seen || s.dist > range * 1.3) return GoalState::Done;
    double v = e.type.speed * 0.6;
    xs = -s.xn * v, ys = -s.yn * v;
    return GoalState::Running;
}
