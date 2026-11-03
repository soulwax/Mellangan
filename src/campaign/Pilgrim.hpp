#pragma once

#include "entity/Entity.hpp"

class Pilgrim : public Entity {
public:
  static constexpr float SPEED = 30, WAIT = 130, HP = 120, ARRIVE = 18;

  Pilgrim(double x, double y, double tx, double ty) : tx(tx), ty(ty) {
    this->x = x, this->y = y;
  }
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
  void renderLight(Screen &s) override;

  double tx, ty;
  float hp = HP, t = 0, flash = 0, facing = 1;
  bool waiting = false;

  bool arrived() const;
};
