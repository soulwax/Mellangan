#pragma once

#include "entity/enemy/Enemy.hpp"

class Thief : public Enemy {
public:
  using Enemy::Enemy;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

protected:
  void die() override;
};
