#pragma once

#include "entity/enemy/ai/Goal.hpp"

class Leap : public Goal {
public:
  Leap(float crouch, float flight, float cooldown, double height)
      : crouch(crouch), flight(flight), cooldown(cooldown), height(height) {}
  void start(Enemy &e, const Sense &s) const override;
  GoalState tick(Enemy &e, const Sense &s, double &xs, double &ys) const override;
  float tell(const Enemy &e) const override;

private:
  float crouch, flight, cooldown;
  double height;
};
