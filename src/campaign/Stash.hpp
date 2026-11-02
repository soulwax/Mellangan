#pragma once

#include "entity/Entity.hpp"

class Level;

class Stash : public Entity {
public:
  static constexpr float REACH = 26;

  Stash(double x, double y) { this->x = x, this->y = y; }
  void onAdded() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;

  static Stash *near(Level &level);
};
