#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Escape : public Goal {
public:
  explicit Escape(float duration = 10.0f) : duration(duration) {}
  GoalState tick(Enemy &e, const Sense &s, double &xs,
                 double &ys) const override;
  float tell(const Enemy &e) const override;

private:
  float duration;
};
