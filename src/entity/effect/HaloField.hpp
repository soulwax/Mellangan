#pragma once

#include "entity/Entity.hpp"

class Halo;
class Player;

class HaloField : public Entity {
public:
  HaloField(const Halo &halo, const Player &p) : halo(halo), player(p) {}
  int layer() const override { return 0; }
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

private:
  const Halo &halo;
  const Player &player;
  float spin = 0;
};
