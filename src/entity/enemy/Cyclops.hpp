#pragma once

#include "entity/enemy/Enemy.hpp"

class Cyclops : public Enemy {
public:
  using Enemy::Enemy;
  void renderLight(Screen &s) override;

protected:
  void die() override;
};
