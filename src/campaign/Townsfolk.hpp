#pragma once

#include "entity/Entity.hpp"

class Townsfolk : public Entity {
public:
  static constexpr float SPEED = 18, HEAR = 34, SAY = 5;

  Townsfolk(double x, double y, double cx, double cy, int tile, int town, int voice)
      : home{cx, cy}, tile(tile), town(town), voice(voice) {
    this->x = x, this->y = y;
  }
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;

  struct {
    double x, y;
  } home;
  int tile, town, voice, said = 0;
  const char *spoken = "";
  double tx = 0, ty = 0;
  float t = 0, wander = 0, talk = 0, hush = 0, facing = 1;

  const char *line() const;
};
