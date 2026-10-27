#pragma once

#include "entity/enemy/Enemy.hpp"

class SupplyBarrel : public Enemy {
public:
  SupplyBarrel();
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

protected:
  void die() override;
};
