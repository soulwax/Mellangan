#pragma once

#include "hero/Hero.hpp"

class Alchemist : public Hero {
public:
  Alchemist();
  void slay(Player &p, Enemy &e) const override;
};
