#pragma once

#include "entity/Entity.hpp"

class Smite : public Entity {
public:
  bool hostile = false;

  Smite(double x, double y, float damage, float radius);
  int layer() const override { return 2; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  static constexpr float BEAM = 0.32f;
  float windup() const { return hostile ? 0.6f : 0.18f; }
  float damage, t = 0;
  bool struck = false;
  void strike();
};
