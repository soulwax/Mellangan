#pragma once

#include "hero/Hero.hpp"

class Priestess : public Hero {
public:
  Priestess();
  void cast(Player &p) const override;
  float devotion() const override { return 1.3f; }
};
