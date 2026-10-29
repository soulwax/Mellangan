#pragma once

#include <atomic>

#include "particle/Particle.hpp"

enum class DeathStyle { Spirit, Flame, Slime, Fallen, Gilded };

class DeathEffect : public Particle {
public:
  inline static std::atomic<int> alive{0};
  DeathStyle style;
  float size, facing, height;
  Color tint;

  DeathEffect(double x, double y, float height, Sprite body, float size,
              float facing, Color tint, DeathStyle style);
  ~DeathEffect() override { alive--; }
  void tick() override;
  void render(Screen &s) override;
};
