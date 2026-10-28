#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Falter : public Goal {
public:
  explicit Falter(float below) : below(below) {}
  bool urgent(const Enemy &e, const Sense &s) const override;
  void start(Enemy &e, const Sense &s) const override;
  GoalState tick(Enemy &e, const Sense &s, double &xs,
                 double &ys) const override;

private:
  float below;
};
