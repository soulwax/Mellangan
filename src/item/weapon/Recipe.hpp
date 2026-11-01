#pragma once

#include <array>
#include <string>
#include <vector>

#include "entity/Entity.hpp"
#include "gfx/Screen.hpp"
#include "item/Upgrade.hpp"

class Enemy;
class FormBehavior;
class Mob;

enum class Form {
  Bolt,
  Volley,
  Orbit,
  Nova,
  Rain,
  Field,
  Seeker,
  Cleave,
  Boomerang,
  Lob,
  Trap,
  Beam
};

namespace pay {
constexpr unsigned Burn = 1, Chill = 2, Poison = 4, Mark = 8, Pull = 16,
                   Pierce = 32, Explode = 64, Split = 128, Leech = 256,
                   Chain = 512, Stagger = 1024;
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
  double ox, oy, dx, dy, tx = 0, ty = 0;
  int index, of, hitsLeft;
  bool child = false, burstOnEnd = false, back = false;
  std::vector<entt::entity> hits;

  const FormBehavior &form() const;
  bool hostile() const { return source && source->hostile(); }
  bool hitBefore(entt::entity id) const;
  Mob *wielder() const;
  void hit(Mob &e, double xd, double yd, float dmg, float knock);
  void strike(Mob &e, double xd, double yd);
  void chain(Mob &from);
  void touch(float r);
  void blast(float radius, float dmg);
  void steer(float range);
};

void fireRecipe(const Recipe &r, Mob &p, Upgrade *source, float damage,
                float reach, int count);
inline void castRecipe(const Recipe &r, Mob &p, Upgrade *source, float power) {
  fireRecipe(r, p, source, r.damage * power, r.reach, r.count);
}
