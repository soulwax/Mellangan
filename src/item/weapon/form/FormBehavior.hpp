#pragma once

#include <cmath>
#include <vector>

#include "item/weapon/Recipe.hpp"

class Mob;
class Screen;
class Upgrade;

inline void unit(double &dx, double &dy) {
  double len = std::hypot(dx, dy);
  if (len < 0.001)
    dx = 1, dy = 0;
  else
    dx /= len, dy /= len;
}

struct FormCast {
  const Recipe &r;
  Mob &p;
  Upgrade *source;
  float damage, reach;
  int count;
  std::vector<Mob *> targets;
  double dx, dy, base;

  RecipeShot *shot(double x, double y, double a, int i) const;
};

class FormBehavior {
public:
  virtual ~FormBehavior() = default;
  static const FormBehavior &of(Form form);

  virtual void fire(const FormCast &c) const = 0;
  virtual float life(const RecipeShot &s) const = 0;
  virtual void tick(RecipeShot &s, Mob &p, float speed) const = 0;
  virtual void render(RecipeShot &s, Screen &screen) const = 0;
  virtual void light(RecipeShot &s, Screen &screen) const;
  virtual void trail(RecipeShot &) const {}
  virtual void expire(RecipeShot &) const {}
  virtual float knock() const { return 40; }
  virtual bool area() const { return false; }
  virtual bool flies() const { return false; }
  virtual bool explodes() const { return true; }

protected:
  static void glint(RecipeShot &s, Screen &screen, float rot);
  static void wake(RecipeShot &s);
};
