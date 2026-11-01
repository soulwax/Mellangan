#pragma once

#include "entity/Entity.hpp"

class Mob;

struct AuraDef {
  float radius, life, heal, damage, chill, poison, burn, mark, zap, bolt;
  bool follow;
  Color tone;
  int particle;
};

class Aura : public Entity {
public:
  Aura(const Mob &owner, const AuraDef &def);
  int layer() const override { return 0; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  entt::entity owner;
  bool hostile;
  AuraDef d;
  float age = 0, pulse = 0, zap = 0;
  int zaps = 0;
  float fade() const;
  void strikeOne();
};
