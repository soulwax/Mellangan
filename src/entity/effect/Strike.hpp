#pragma once

#include <array>

#include "entity/Entity.hpp"

class Strike : public Entity {
public:
  Strike(double x, double y, float damage, float radius);
  int layer() const override { return 2; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;
  bool hostile = false;

private:
  float damage, life = 0.3f;
  bool struck = false;
  std::array<float, 7> jag{};
  std::array<float, 3> fork{};
};
