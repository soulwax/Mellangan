#pragma once

#include <memory>

#include "entity/Mob.hpp"
#include "entity/enemy/ai/Goal.hpp"

class Sound;

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
  Cyclops,
  Wolf,
  Alpha,
  Burrower,
  Necromancer,
  Skeleton,
  Thief,
  MireLeech,
  BogLantern,
  GlassMantis,
  DustVulture,
  FrostWisp,
  Iceback,
  Sporeling,
  MycelialBrute,
  EmberImp,
  CinderGolem,
  Count
};

struct EnemyType {
  int tile;
  float hp, speed, damage, radius, scale, xp, knockResist;
  Color blood;
  bool floats;
  float unlockAt;
  Sprite (*art)(int) = nullptr;
};

class Enemy : public Mob {
public:
  const EnemyType &type;
  float damageMul, knifeCooldown = 0;
  bool elite = false;
  const Plan *plan = nullptr;
  GoalQueue goals;
  float goalTime = 0, cooldown = 0, frenzy = 0, age = 0;
  double xMark = 0, yMark = 0, xWant = 0, yWant = 0, xPush = 0, yPush = 0,
         side = 1;
  entt::entity leader = entt::null;
  uint32_t seed;
  bool aware = true, wantsFire = false, wantsLeader = false, faltered = false,
       alarmed = false, buried = false, gilded = false, boss = false;

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
            Upgrade *src = nullptr, bool periodic = false);
  const char *name() const;
  int kind() const;
  bool is(EnemyKind k) const { return kind() == int(k); }
  Sprite sprite() const;
  void makeElite(float health = 4);
  bool hostile() const override { return true; }
  void split() override;
  void makeBoss(float health, float damage);
  void think();
  void move();
  void push();
  void settle();
  float roll();
  void voice(Sound &s, float volume, float jitter = 0.06f) const;

protected:
  virtual Color tint() const { return col::white; }
  virtual void die();

private:
  void act();
  void adopt();
  Sense sense() const;
  void next(const Sense &s);
};
