#include "entity/enemy/ai/Approach.hpp"

#include "entity/enemy/Enemy.hpp"

GoalState Approach::tick(Enemy &e, const Sense &s, double &xs, double &ys) const {
    e.aware = true;
    if (stop > 0 && s.dist < stop && e.cooldown <= 0) return GoalState::Done;
    double wave = std::sin(e.animT * gait.pulseRate);
    double v = e.type.speed * (gait.base + gait.pulse * (gait.rectified ? std::max(0.0, wave) : wave));
    double w = std::sin(e.animT * gait.weaveRate) * gait.weave;
    xs = s.xn * v - s.yn * w, ys = s.yn * v + s.xn * w;
    return GoalState::Running;
}
