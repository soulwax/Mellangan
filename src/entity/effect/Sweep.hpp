#pragma once

#include <vector>

#include "entity/Entity.hpp"

class Mob;

enum class SweepStyle { Reap, Steel, Rally };

class Sweep : public Entity {
public:
  Sweep(const Mob &owner, float reach, float damage,
        SweepStyle style = SweepStyle::Reap);
  int layer() const override { return 2; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  entt::entity owner;
  bool hostile;
  double cx, cy;
  float reach, damage, angle, start, t = 0;
  SweepStyle style;
  std::vector<entt::entity> hits;
  Color tone() const;
  Sprite blade() const;
  void trail();
};
