#pragma once

#include "particle/debris/Debris.hpp"

class BloodDebris : public Debris {
public:
  BloodDebris(double x, double y, double z, Color c);
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
};
