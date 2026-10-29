#pragma once

#include "particle/debris/Debris.hpp"

class Chip : public Debris {
public:
  float rot, spin;

  Chip(double x, double y, double z, int icon, Color color, double speed);
  void tick() override;
  void render(Screen &s) override;
};
