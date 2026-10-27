#pragma once

#include "entity/Entity.hpp"

class Player;

class Pickup : public Entity {
public:
  float t = 0, speed = 0;
  bool magnet = false;

  Pickup(double x, double y) { this->x = x, this->y = y; }
  int layer() const override { return 0; }
  void tick() override;

protected:
  virtual float reach() const;
  virtual void take(Player &p) = 0;
};
