#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Mend : public Goal {
public:
  Mend(float range, float seconds) : range(range), seconds(seconds) {}
  GoalState tick(Enemy &e, const Sense &s, double &xs,
                 double &ys) const override;

private:
  float range, seconds;
};

Enemy *hurtAlly(Enemy &e, float range);
