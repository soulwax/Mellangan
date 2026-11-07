#pragma once

#include "entity/enemy/Enemy.hpp"

class SynodFoe : public Enemy {
public:
  enum Role { Lamplighter, Penitent, Censer, Limer, Confessor, Prioress };

  static constexpr float SMOKE = 56, PREACH = 90, LIME_EVERY = 2.5f, LIME_LIFE = 25;

  int role;
  float timer = 1;
  float baseDamage;

  SynodFoe(const EnemyType &type, float hpMul, float damageMul, int role);
  void tick() override;
  void renderLight(Screen &s) override;

private:
  void lime();
  void preach();
  void smoke();
};
