#pragma once

#include "entity/pickup/Pickup.hpp"

class Potion : public Pickup {
public:
  using Pickup::Pickup;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

protected:
  float reach() const override;
  void take(Player &p) override;
};
