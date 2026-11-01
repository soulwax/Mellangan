#pragma once

#include "entity/enemy/Enemy.hpp"

struct WildDef {
  Cast cast;
  int casts;
  float grow;
  bool dims, backstab, bramble, mends;
};

class WildFoe : public Enemy {
public:
  static constexpr EnemyKind FIRST = EnemyKind::Thornling, LAST = EnemyKind::Cutthroat;
  static const WildDef DEFS[int(LAST) - int(FIRST) + 1];

  WildFoe(const EnemyType &type, float hpMul, float damageMul, int index);
  void tick() override;
  float contactBonus(const Player &p) const override;

  const WildDef &wild;
  float life = 0, mendTime = 0;

protected:
  void die() override;
};
