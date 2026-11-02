#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Guard : public Goal {
public:
  Guard(float seconds, double pace, double close) : seconds(seconds), pace(pace), close(close) {}
  GoalState tick(Enemy &e, const Sense &s, double &xs, double &ys) const override;

private:
  float seconds;
  double pace, close;
};

class Totem : public Goal {
public:
  GoalState tick(Enemy &e, const Sense &s, double &xs, double &ys) const override;
};
