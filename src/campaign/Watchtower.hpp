#pragma once

#include "entity/Entity.hpp"

class Watchtower : public Entity {
public:
  static constexpr float RANGE = 190, RELOAD = 1.1f;

  Watchtower(double x, double y) { this->x = x, this->y = y; }
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
  void renderLight(Screen &s) override;

  float reload = 0.5f, t = 0;
  int shots = 0;
};
