#pragma once

#include "entity/enemy/ai/Goal.hpp"

class LookOut : public Goal {
public:
    explicit LookOut(double sight) : sight(sight) {}
    void start(Enemy &e, const Sense &s) const override;
    GoalState tick(Enemy &e, const Sense &s, double &xs, double &ys) const override;

private:
    double sight;
};
