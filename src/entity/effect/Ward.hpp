#pragma once

#include "entity/Entity.hpp"

class Player;

class Ward : public Entity {
public:
  Ward(const Player &p, float life) : player(p), life(life), maxLife(life) {}
  int layer() const override { return 2; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  const Player &player;
  float life, maxLife, spin = 0;
  float fade() const;
};
