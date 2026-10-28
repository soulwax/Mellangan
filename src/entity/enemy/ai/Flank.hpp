#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Flank : public Goal {
public:
  explicit Flank(double prefer = 65) : prefer(prefer) {}
  void start(Enemy &e, const Sense &s) const override;
  GoalState tick(Enemy &e, const Sense &s, double &xs,
                 double &ys) const override;

private:
  double prefer;
};
