#pragma once

#include "entity/enemy/Enemy.hpp"

class Wolf : public Enemy {
public:
  using Enemy::Enemy;

protected:
  Color tint() const override;
};
