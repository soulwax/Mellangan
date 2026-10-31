#pragma once

#include <memory>
#include <vector>

#include "entity/Mob.hpp"

class Upgrade;
class Skill;

class Hero;

class Player : public Mob {
public:
  int lvl = 1, pendingLevels = 0, pendingTreasures = 0, revives = 0;
  const char *felledBy = nullptr;
  float exp = 0, expNext = 5, xpBase = 6, xpGrowth = 1.08f;
  bool god = false;
  int tile = atlas::t::Wizard;
  const Hero *hero = nullptr;
  Skill *skill = nullptr;

  Player();
  ~Player() override;

  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
  void renderLight(Screen &s) override;

  void hurt(float damage, bool periodic = false);
  void rise();
  void cast();
  void heal(float amount) { mend(amount); }
  Player *asPlayer() override { return this; }
  void strike(float damage, double, double, float, Upgrade * = nullptr) override {
    hurt(damage);
  }
  void mend(float amount) override;
  void shield(float seconds) override { invuln = std::max(invuln, seconds); }
  Sprite body() const override;
  void gainXp(float amount);
  double torchX() const { return x + facing * 6; }
};
