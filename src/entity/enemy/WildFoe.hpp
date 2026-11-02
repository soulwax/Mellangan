#pragma once

#include "entity/enemy/Enemy.hpp"

struct WildDef {
  Cast cast;
  int casts;
  float grow;
  bool dims, backstab, bramble, mends, chills, guarded, waterborn, lurks, roots, puffs, aura,
      drains, reflects, shatters, nocturnal, blinks;
};

class WildFoe : public Enemy {
public:
  static constexpr EnemyKind FIRST = EnemyKind::Thornling, LAST = EnemyKind::BarrowArcher;
  static const WildDef DEFS[int(LAST) - int(FIRST) + 1];

  WildFoe(const EnemyType &type, float hpMul, float damageMul, int index);
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
  float contactBonus(const Player &p) const override;
  float shell(double xd, double yd) override;
  Color tint() const override;
  void bit(Player &p) override;

  const WildDef &wild;
  float life = 0, mendTime = 0, rootTime = 1, drainTime = 0, drainCool = 1.5f, drainTick = 0,
        glintCool = 0, blinkTime = 2;
  bool glint = false;
  double xHome = 0, yHome = 0;

protected:
  void die() override;

private:
  void surface();
  void strangle(Player &p);
  void puff(Player &p);
  void leech(Player &p, double d);
  void blink(Player &p, double d);
  void reflect();
};
