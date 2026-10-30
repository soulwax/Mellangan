#pragma once

#include <vector>

#include "entity/Entity.hpp"

class Player;

enum class SweepStyle { Reap, Steel, Rally };

class Sweep : public Entity {
public:
  Sweep(const Player &p, float reach, float damage,
        SweepStyle style = SweepStyle::Reap);
  int layer() const override { return 2; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  const Player &player;
  float reach, damage, angle, start, t = 0;
  SweepStyle style;
  std::vector<entt::entity> hits;
  Color tone() const;
  Sprite blade() const;
  void trail();
};
