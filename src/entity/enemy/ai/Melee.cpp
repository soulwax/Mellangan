#include "entity/enemy/ai/Melee.hpp"

#include "entity/enemy/Enemy.hpp"

void Melee::start(Enemy &e, const Sense &s) const { e.xMark = s.xn, e.yMark = s.yn; }

GoalState Melee::tick(Enemy &e, const Sense &, double &xs, double &ys) const {
    if (e.goalTime < windup) {
        xs = -e.xMark * 8, ys = -e.yMark * 8;
        return GoalState::Running;
    }
    if (e.goalTime < windup + lunge) {
        xs = e.xMark * e.type.speed * speed, ys = e.yMark * e.type.speed * speed;
        return GoalState::Running;
    }
    e.cooldown = cooldown;
    return GoalState::Done;
}

float Melee::tell(const Enemy &e) const { return e.goalTime < windup ? e.goalTime / windup : 0; }
