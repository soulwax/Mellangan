#pragma once

#include "entity/Entity.hpp"

class Level;

class Merchant : public Entity {
public:
  static constexpr float REACH = 36;

  Merchant(double x, double y) { this->x = x, this->y = y; }
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
  void renderLight(Screen &s) override;

  float t = 0, facing = 1;

  static void visit(Level &level);
  static Merchant *camp(Level &level, double x, double y);
  static Merchant *near(Level &level);
};
