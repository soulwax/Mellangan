#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Stalk : public Goal {
public:
    explicit Stalk(double range) : range(range) {}
    GoalState tick(Enemy &e, const Sense &s, double &xs, double &ys) const override;

private:
    double range;
};
