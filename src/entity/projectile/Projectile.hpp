#pragma once

#include <vector>

#include "entity/Entity.hpp"

class Mob;

class Projectile : public Entity {
public:
  float life, maxLife, damage, knock;
  int pierce;
  bool hostile = false;
  std::vector<entt::entity> hits;

  Projectile(double x, double y, double xa, double ya, float life, float damage,
             float knock, int pierce)
      : life(life), maxLife(life), damage(damage), knock(knock),
        pierce(pierce) {
    this->x = x, this->y = y;
    this->xa = xa, this->ya = ya;
  }
  int layer() const override { return 2; }

protected:
  void collide();
  virtual bool touches(const Mob &e) const;
  virtual void push(const Mob &, double &xd, double &yd) const {
    xd = xa, yd = ya;
  }
  virtual void spent() {}
  float frac() const { return life / maxLife; }
};
