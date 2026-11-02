#pragma once

#include "entity/Entity.hpp"

class Level;

class Waypoint : public Entity {
public:
  static constexpr float REACH = 22;

  Waypoint(double x, double y) { this->x = x, this->y = y; }
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
  void renderLight(Screen &s) override;

  float t = 0;
  bool inside = true;

  static Waypoint *near(Level &level);
};
