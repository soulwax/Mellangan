#pragma once

#include "hero/Hero.hpp"

class Reaper : public Hero {
public:
  Reaper();
  void slay(Player &p, Enemy &e) const override;
};
