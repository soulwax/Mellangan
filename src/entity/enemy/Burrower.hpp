#pragma once

#include "entity/enemy/Enemy.hpp"

class Burrower : public Enemy {
public:
  using Enemy::Enemy;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
  void renderLight(Screen &s) override;
};
