#pragma once

#include "entity/Entity.hpp"

class Level;

class Elder : public Entity {
public:
  static constexpr float REACH = 26;

  Elder(double x, double y) { this->x = x, this->y = y; }
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;

  float t = 0, facing = 1;

  static Elder *near(Level &level);
};
