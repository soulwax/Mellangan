#pragma once

#include "entity/enemy/Enemy.hpp"

class Mimic : public Enemy {
public:
  Mimic(const EnemyType &type, float hpMul, float damageMul)
      : Enemy(type, hpMul, damageMul) {
    buried = true;
  }
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
  void renderLight(Screen &s) override;

protected:
  void die() override;
};
