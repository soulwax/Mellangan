#pragma once

#include "entity/Entity.hpp"

class Enemy;

class Sentry : public Entity {
public:
  static constexpr float GUARD = 120, REACH = 16, SPEED = 64, SWING = 0.4f, DAMAGE = 18,
                         KNOCK = 150, BODY = 9, CLEAVE = 24;
  static constexpr int PER_TOWN = 4;

  Sentry(double x, double y, int tile) : post{x, y}, tile(tile) { this->x = x, this->y = y; }
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;

  struct {
    double x, y;
  } post;
  int tile, kills = 0;
  float swing = 0, t = 0, facing = 1, lunge = 0;

private:
  Enemy *target() const;
  void shove();
  void cleave();
  float might() const;
};

class Bridge : public Entity {
public:
  Bridge(double x, double y, bool across) : across(across) { this->x = x, this->y = y; }
  int layer() const override { return 0; }
  void render(Screen &s) override;

  bool across;
};
