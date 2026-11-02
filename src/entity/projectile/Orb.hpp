#pragma once

#include "entity/Entity.hpp"

class Orb : public Entity {
public:
  const char *by = nullptr;
  Color tone{0.75f, 0.35f, 1, 1};
  float chill = 0, size = 1.2f;

  Orb(double x, double y, double xa, double ya, float damage) : damage(damage) {
    this->x = x, this->y = y;
    this->xa = xa, this->ya = ya;
  }
  int layer() const override { return 2; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  float damage, life = 5;
};
