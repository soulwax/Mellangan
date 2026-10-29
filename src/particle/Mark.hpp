#pragma once

#include "particle/Particle.hpp"

class Mark : public Particle {
public:
  int icon;
  Sprite art;
  Color color;
  float scale, rot;

  Mark(double x, double y, float life, int icon, Color color, float scale = 1,
       float rot = 0)
      : Particle(x, y, life), icon(icon), art{}, color(color), scale(scale),
        rot(rot) {}
  Mark(double x, double y, float life, Sprite art, Color color, float scale)
      : Particle(x, y, life), icon(-1), art(art), color(color), scale(scale),
        rot(0) {}
  int layer() const override { return 0; }
  void render(Screen &s) override;
};
