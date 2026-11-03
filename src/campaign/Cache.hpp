#pragma once

#include "entity/Entity.hpp"

class Cache : public Entity {
public:
  static constexpr float REACH = 16;

  Cache(double x, double y) { this->x = x, this->y = y; }
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
  void renderLight(Screen &s) override;

  float t = 0;
  bool sealed = true;
};
