#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Burrow : public Goal {
public:
  explicit Burrow(float travel = 2.8f, float warn = 0.75f, float surface = 2.5f)
      : travel(travel), warn(warn), surface(surface) {}
  void start(Enemy &e, const Sense &s) const override;
  GoalState tick(Enemy &e, const Sense &s, double &xs,
                 double &ys) const override;
  float tell(const Enemy &e) const override;
  bool conceals() const override { return true; }

private:
  float travel;
  float warn;
  float surface;
};
