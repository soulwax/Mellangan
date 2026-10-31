#pragma once

#include <array>
#include <string>
#include <vector>

#include "entity/Entity.hpp"
#include "gfx/Screen.hpp"
#include "item/Upgrade.hpp"

class Enemy;

enum class Form { Bolt, Volley, Orbit, Nova, Rain, Field, Seeker };

namespace pay {
constexpr unsigned Burn = 1, Chill = 2, Poison = 4, Mark = 8, Pull = 16,
                   Pierce = 32, Explode = 64, Split = 128;
}

struct Recipe {
  const char *name, *owner, *pitch, *unit;
  int icon;
  Form form;
  unsigned payload;
  float damage, cooldown, reach, speed;
  int count;
  Color tint;
  int particle;
};

extern const std::vector<Recipe> RECIPES;

class RecipeWeapon : public Upgrade {
public:
  explicit RecipeWeapon(const Recipe &r);
  void tick(Mob &p) override;
  int count() const;
  float reach() const;
  float damage(const Mob &p) const;

  const Recipe &r;
  float cd = 0.5f;

private:
  std::array<std::string, MAX_LEVEL> text;
};

class RecipeShot : public Entity {
public:
  RecipeShot(const Recipe &r, double x, double y, double dx, double dy,
             float damage, float reach, int index = 0, int of = 1);
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;
  int layer() const override { return 2; }

  const Recipe &r;
  float age = 0, life, damage, reach, pulse = 0;
  double ox, oy, dx, dy;
  int index, of, hitsLeft;
  bool child = false, burstOnEnd = false;
  std::vector<entt::entity> hits;

private:
  bool hitBefore(entt::entity id) const;
  void strike(Mob &e, double xd, double yd);
  void blast(float radius, float dmg);
  void trail();
  void steer(float range);
};

void fireRecipe(const Recipe &r, Mob &p, Upgrade *source, float damage,
                float reach, int count);
