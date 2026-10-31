#pragma once

#include "entity/Mob.hpp"

class Knives;

class Knife : public Entity {
public:
  Knife(const Knives &knives, const Mob &p, int index)
      : knives(knives), owner(p), ownerId(p.id), index(index) {}
  int layer() const override { return 2; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  const Knives &knives;
  const Mob &owner;
  entt::entity ownerId;
  int index;
  float angle = 0;
};
