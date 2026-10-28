#pragma once

#include "hero/Hero.hpp"

class ExpansionHero : public Hero {
public:
  int variant;

  explicit ExpansionHero(int variant);
  Sprite sprite() const override;
  void equip(Player &p) const override;
  void cast(Player &p) const override;
  void slay(Player &p, Enemy &e) const override;
  float wisdom() const override { return variant == 2 ? 1.15f : 1.0f; }
  float edge() const override { return variant == 3 ? 2.2f : 1.8f; }
};
