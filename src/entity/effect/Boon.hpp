#pragma once

#include "entity/Entity.hpp"
#include "entity/Mob.hpp"

class Boon : public Entity {
public:
  Boon(const Mob &target, float Mob::*stat, float delta, float seconds, float delay,
       Color tone);
  void tick() override;
  void render(Screen &) override {}

private:
  entt::entity target;
  float Mob::*stat;
  float delta, life, delay;
  Color tone;
  bool on = false;
};
