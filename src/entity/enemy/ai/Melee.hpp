#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Melee : public Goal {
public:
    Melee(float windup, float lunge, double speed, float cooldown) : windup(windup), lunge(lunge), speed(speed), cooldown(cooldown) {}
    void start(Enemy &e, const Sense &s) const override;
    GoalState tick(Enemy &e, const Sense &s, double &xs, double &ys) const override;
    float tell(const Enemy &e) const override;

private:
    float windup, lunge;
    double speed;
    float cooldown;
};
