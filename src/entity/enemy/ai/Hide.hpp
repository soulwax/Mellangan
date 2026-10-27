#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Hide : public Goal {
public:
  explicit Hide(double range) : range(range) {}
  bool urgent(const Enemy &e, const Sense &s) const override;
  GoalState tick(Enemy &e, const Sense &s, double &xs,
                 double &ys) const override;
  bool conceals() const override { return true; }

private:
  double range;
};
