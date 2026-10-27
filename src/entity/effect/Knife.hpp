#pragma once

#include "entity/Entity.hpp"

class Knives;
class Player;

class Knife : public Entity {
public:
  Knife(const Knives &knives, const Player &p, int index)
      : knives(knives), player(p), index(index) {}
  int layer() const override { return 2; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  const Knives &knives;
  const Player &player;
  int index;
  float angle = 0;
};
