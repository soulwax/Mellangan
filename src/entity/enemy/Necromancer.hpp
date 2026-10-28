#pragma once

#include "entity/enemy/Enemy.hpp"

class Necromancer : public Enemy {
public:
  using Enemy::Enemy;
  void renderLight(Screen &s) override;
};
