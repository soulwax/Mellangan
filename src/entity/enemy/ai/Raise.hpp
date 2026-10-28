#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Raise : public Goal {
public:
  explicit Raise(float windup = 0.85f, float cooldown = 6.0f)
      : windup(windup), cooldown(cooldown) {}
  GoalState tick(Enemy &e, const Sense &s, double &xs,
                 double &ys) const override;
  float tell(const Enemy &e) const override;

private:
  float windup;
  float cooldown;
};
