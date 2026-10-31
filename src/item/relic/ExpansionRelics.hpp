#pragma once

#include "item/Upgrade.hpp"

class ExpansionRelic : public Upgrade {
public:
  explicit ExpansionRelic(int kind);
  void tick(Mob &p) override;
  void slay(Mob &p, Enemy &e) override;
  float guard(Mob &p, float damage) override;
  void cast(Mob &p) override;
  int kind, claims = 0;
  float cd = 0, delay = 0, stored = 0;

private:
  void pulse(Mob &p, double x, double y, float damage, int status);
};
