#pragma once

#include <memory>

#include "entity/Mob.hpp"
#include "entity/enemy/ai/Goal.hpp"

enum class EnemyKind {
  Bat,
  Rat,
  Slime,
  Scorpion,
  Spider,
  Ghost,
  Cultist,
  Demon,
  Knight,
  Cyclops
};

struct EnemyType {
  int tile;
  float hp, speed, damage, radius, scale, xp, knockResist;
  Color blood;
  bool floats;
  float unlockAt;
};

class Enemy : public Mob {
public:
  const EnemyType &type;
  float damageMul, knifeCooldown = 0;
  bool elite = false;
  const Plan *plan = nullptr;
  GoalQueue goals;
  float goalTime = 0, cooldown = 0;
  double xMark = 0, yMark = 0, xWant = 0, yWant = 0, xPush = 0, yPush = 0,
         side = 1;
  entt::entity leader = entt::null;
  uint32_t seed;
  bool aware = true, wantsFire = false, wantsLeader = false, faltered = false,
       alarmed = false;

  Enemy(const EnemyType &type, float hpMul, float damageMul);
  static const EnemyType &typeOf(EnemyKind kind);
  static std::unique_ptr<Enemy> create(EnemyKind kind, float hpMul,
                                       float damageMul);

  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
  float occlusion(float &width) const override {
    return type.floats ? (width = 0, 0.0f) : Mob::occlusion(width);
  }
  void renderLight(Screen &s) override;
  void hurt(float damage, double xd, double yd, float force,
            Upgrade *source = nullptr);
  const char *name() const;
  int kind() const;
  void makeElite();
  void think();
  void move();
  void push();
  void settle();
  float roll();

protected:
  virtual Color tint() const { return col::white; }
  virtual void die();

private:
  void act();
  void adopt();
  Sense sense() const;
  void next(const Sense &s);
};
