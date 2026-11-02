#pragma once

#include "entity/enemy/Enemy.hpp"

struct WildDef {
  Cast cast;
  int casts;
  float grow;
  bool dims, backstab, bramble, mends, chills, guarded, waterborn, lurks, roots, puffs, aura,
      drains, reflects, shatters, nocturnal, blinks, wyrm, skittish, salty, totem, trail, veil,
      scorches, blind, drake, stalker, robber, clings, ore, pilfers;
};

class WildFoe : public Enemy {
public:
  static constexpr EnemyKind FIRST = EnemyKind::Thornling, LAST = EnemyKind::KoboldMiner;
  static const WildDef DEFS[int(LAST) - int(FIRST) + 1];

  WildFoe(const EnemyType &type, float hpMul, float damageMul, int index);
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
  float contactBonus(const Player &p) const override;
  float shell(double xd, double yd) override;
  Color tint() const override;
  bool veiled() const;
  void bit(Player &p) override;

  const WildDef &wild;
  float life = 0, mendTime = 0, rootTime = 1, drainTime = 0, drainCool = 1.5f, drainTick = 0,
        glintCool = 0, blinkTime = 2, wyrmTime = 1, saltTime = 0, pulseTime = 1;
  float flightTime = 10, restTime = 0, breathTime = 2, heard = -1;
  double xRest = 0, yRest = 0;
  int eruptions = 0, pulses = 0, stolen = 0;
  bool glint = false, fled = false;
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
  void burrow(Player &p);
  void pulse();
  void listen();
  void fly(Player &p, double d);
  void breathe();
  void bequeath();
};
