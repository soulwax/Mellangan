#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Circle : public Goal {
public:
  Circle(double radius, float seconds) : radius(radius), seconds(seconds) {}
  GoalState tick(Enemy &e, const Sense &s, double &xs,
                 double &ys) const override;

private:
  double radius;
  float seconds;
};
