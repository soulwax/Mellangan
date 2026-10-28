#pragma once

#include "hero/Hero.hpp"

class Alchemist : public Hero {
public:
  Alchemist();
  void cast(Player &p) const override;
  void slay(Player &p, Enemy &e) const override;
};
