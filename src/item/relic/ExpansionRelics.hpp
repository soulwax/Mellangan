#pragma once

#include "item/Upgrade.hpp"

class ExpansionRelic : public Upgrade {
public:
  explicit ExpansionRelic(int kind);
  void tick(Player &p) override;
  void slay(Player &p, Enemy &e) override;
  float guard(Player &p, float damage) override;
  void cast(Player &p) override;
  int kind, claims = 0;
  float cd = 0, delay = 0, stored = 0;

private:
  void pulse(Player &p, double x, double y, float damage, int status);
};
