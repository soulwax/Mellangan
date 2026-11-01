#pragma once

#include "hero/Hero.hpp"

class ExpansionHero : public Hero {
public:
  ExpansionHero(const HeroData &data, int art) : Hero(data), art(art) {}
  Sprite sprite() const override { return atlas::expansion(10 + art); }

private:
  int art;
};
