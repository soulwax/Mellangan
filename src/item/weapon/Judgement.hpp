#pragma once

#include "item/Upgrade.hpp"

class Judgement : public Upgrade {
public:
  Judgement();
  void tick(Player &p) override;

private:
  float cd = 1;
};
