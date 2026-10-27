#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Ranged : public Goal {
public:
    Ranged(double prefer, double range) : prefer(prefer), range(range) {}
    GoalState tick(Enemy &e, const Sense &s, double &xs, double &ys) const override;

private:
    double prefer, range;
};
