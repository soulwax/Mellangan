#pragma once

#include "entity/enemy/ai/Goal.hpp"

struct Gait {
    double base = 1, pulse = 0, pulseRate = 0, weave = 0, weaveRate = 0;
    bool rectified = false;
};

class Approach : public Goal {
public:
    Approach(double stop, Gait gait) : stop(stop), gait(gait) {}
    GoalState tick(Enemy &e, const Sense &s, double &xs, double &ys) const override;

private:
    double stop;
    Gait gait;
};
