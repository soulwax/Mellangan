#pragma once

#include "entity/enemy/Enemy.hpp"

class Ghost : public Enemy {
public:
  using Enemy::Enemy;
  void renderLight(Screen &s) override;

protected:
  Color tint() const override;
};
