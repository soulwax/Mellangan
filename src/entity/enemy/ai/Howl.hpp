#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Howl : public Goal {
public:
  explicit Howl(float windup = 0.6f, float cooldown = 7.5f)
      : windup(windup), cooldown(cooldown) {}
  GoalState tick(Enemy &e, const Sense &s, double &xs,
                 double &ys) const override;
  float tell(const Enemy &e) const override;

private:
  float windup;
  float cooldown;
};
