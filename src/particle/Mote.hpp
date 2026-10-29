#pragma once

#include "particle/Particle.hpp"

class Mote : public Particle {
public:
  int icon;
  Color color;

  Mote(double x, double y, int icon, Color color, float life);
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  double x0, y0, bend;
  float heading = 0;
};
