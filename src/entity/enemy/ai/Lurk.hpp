#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Lurk : public Goal {
public:
  explicit Lurk(double wake) : wake(wake) {}
  GoalState tick(Enemy &e, const Sense &s, double &xs,
                 double &ys) const override;
  bool conceals() const override { return true; }

private:
  double wake;
};
