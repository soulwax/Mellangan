#pragma once

#include "entity/enemy/Enemy.hpp"

class ExpansionEnemy : public Enemy {
public:
  enum Variant { Leech, Lantern, Mantis, Vulture, Wisp, Iceback, Sporeling, Brute, Imp, Golem };

  int variant, state = 0;
  float timer = 0, pulse = 0;
  double aimX = 1, aimY = 0;

  ExpansionEnemy(const EnemyType &type, float hpMul, float damageMul,
                 int variant);
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

protected:
  void die() override;

private:
  void windup(float seconds, double xd, double yd, double distance);
  void burst(float reach, float damage, StatusKind kind, float power);
  void hazard(float radius, float life, float delay, float damage);
  void mend();
  void stalk(Player &p, double xd, double yd, double dist);
  void release();
  void charge(Player &p, double dist);
};
