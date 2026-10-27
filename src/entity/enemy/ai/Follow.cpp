#include "entity/enemy/ai/Follow.hpp"

#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"

GoalState Follow::tick(Enemy &e, const Sense &s, double &xs, double &ys) const {
    e.aware = true;
    if (s.dist < breakOff) return GoalState::Done;
    const Enemy *lead = e.level->get<Enemy>(e.leader);
    if (!lead) {
        e.wantsLeader = true;
        if (e.goalTime > 1) return GoalState::Done;
        xs = s.xn * e.type.speed, ys = s.yn * e.type.speed;
        return GoalState::Running;
    }
    double tx = lead->x - s.yn * 10 * e.side - s.xn * 12, ty = lead->y + s.xn * 10 * e.side - s.yn * 12;
    double xd = tx - e.x, yd = ty - e.y, dd = std::sqrt(xd * xd + yd * yd);
    double v = e.type.speed * std::min(1.2, dd / 16);
    if (dd > 1e-3) xs = xd / dd * v, ys = yd / dd * v;
    else xs = ys = 0;
    return GoalState::Running;
}
