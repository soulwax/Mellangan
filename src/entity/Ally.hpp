#pragma once

#include "entity/Mob.hpp"
#include "entity/enemy/Enemy.hpp"

struct Recipe;

enum class AllyKind { Wolf, Hawk, Ghoul, Skeleton, Drone, Crew, Decoy, Ghost, Count };

struct AllyDef {
  const char *name, *recipe;
  EnemyKind look;
  float hp, speed, every, reach, life, scale;
  bool flies, lure;
  Color tint;
  const Recipe *shot;
};

const AllyDef &allyDef(AllyKind kind);
AllyKind allyFor(const char *recipe);

class Ally : public Mob {
public:
  static constexpr float LEASH = 170, SIGHT = 150, HOME = 24, GRACE = 0.4f;
  const AllyDef &def;
  float damage, life, bite = 0, age = 0;
  unsigned payload = 0;
  int slot = 0;

  Ally(AllyKind kind, double x, double y, float damage, float life, unsigned payload = 0);
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
  void strike(float damage, double xd, double yd, float force, Upgrade * = nullptr) override;
  Sprite body() const override;
  void retire();

private:
  Mob *pick() const;
  void stride(double tx, double ty, float pace);
  void bitten();
  void blast(const Mob &at);
};

Ally *summon(Level &level, AllyKind kind, double x, double y, float damage, float life,
             unsigned payload = 0, Upgrade *source = nullptr);
