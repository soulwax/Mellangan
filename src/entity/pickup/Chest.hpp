#pragma once

#include "entity/pickup/Pickup.hpp"

class Chest : public Pickup {
public:
  using Pickup::Pickup;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

protected:
  void take(Player &p) override;
};
