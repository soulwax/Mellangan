#pragma once

#include "entity/Mob.hpp"

class Halo;

class HaloField : public Entity {
public:
  HaloField(const Halo &halo, const Mob &p) : halo(halo), owner(p), ownerId(p.id) {}
  int layer() const override { return 0; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  const Halo &halo;
  const Mob &owner;
  entt::entity ownerId;
  float spin = 0;
};
