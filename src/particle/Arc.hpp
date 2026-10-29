#pragma once

#include <vector>

#include "particle/Particle.hpp"

struct ArcNode {
  double x, y;
};

class Arc : public Particle {
public:
  std::vector<ArcNode> nodes;
  Color color;

  Arc(std::vector<ArcNode> nodes, float life, Color color);
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  uint32_t seed;
  float flicker = 0;
};
