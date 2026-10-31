#pragma once

#include "entity/enemy/Enemy.hpp"

struct Arms {
  const char *weapon;
  Cast cast;
};

class Renegade : public Enemy {
public:
  static constexpr int COUNT = 4;
  static const Arms ARMS[COUNT];

  Renegade(const EnemyType &type, float hpMul, float damageMul, int index);
  void onAdded() override;

  const Arms &arms;

protected:
  void die() override;
};
