#pragma once

#include "hero/Hero.hpp"

class Stormcaller : public Hero {
public:
  Stormcaller();
  void slay(Player &p, Enemy &e) const override;
};
