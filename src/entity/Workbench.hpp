#pragma once

#include "entity/Entity.hpp"

class Level;

class Workbench : public Entity {
public:
  static constexpr float REACH = 30;

  Workbench(double x, double y) { this->x = x, this->y = y; }
  void onAdded() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;

  static Workbench *near(Level &level);
};
