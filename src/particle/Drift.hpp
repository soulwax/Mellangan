#pragma once

#include <atomic>

#include "particle/Particle.hpp"

class Drift : public Particle {
public:
  inline static std::atomic<int> alive{0};
  static constexpr int MAX = 360;

  int icon;
  Color color;
  float rot, spin = 0, sway = 0, grow = 0, scale = 1, gravity = 0, drag = 2;

  Drift(double x, double y, double xa, double ya, int icon, Color color,
        float life);
  ~Drift() override { alive--; }
  static bool room() { return alive < MAX; }
  void tick() override;
  void render(Screen &s) override;
};
