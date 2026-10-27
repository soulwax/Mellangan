#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Follow : public Goal {
public:
    explicit Follow(double breakOff) : breakOff(breakOff) {}
    GoalState tick(Enemy &e, const Sense &s, double &xs, double &ys) const override;

private:
    double breakOff;
};
