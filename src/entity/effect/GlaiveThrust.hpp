#pragma once

#include <vector>

#include "entity/Entity.hpp"

class Player;

class GlaiveThrust : public Entity {
public:
  GlaiveThrust(const Player &p, double dirX, double dirY, float reach,
               float damage, float force, bool doubleEnded);
  int layer() const override { return 2; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  const Player &player;
  double dx, dy;
  float reach, damage, force, t = 0;
  bool doubleEnded;
  std::vector<entt::entity> hits;
};

