#pragma once

#include "hero/Hero.hpp"

class Stormcaller : public Hero {
public:
  Stormcaller();
  void cast(Player &p) const override;
  void slay(Player &p, Enemy &e) const override;
};
