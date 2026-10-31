#pragma once

#include "entity/Entity.hpp"

class Mob;

class Ward : public Entity {
public:
  Ward(const Mob &owner, float life);
  int layer() const override { return 2; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  entt::entity owner;
  double cy;
  float life, maxLife, spin = 0;
  float fade() const;
};
