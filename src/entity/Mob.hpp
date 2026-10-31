#pragma once

#include "entity/Entity.hpp"
#include "entity/Status.hpp"
#include "entity/Trait.hpp"

#include <memory>
#include <vector>

class Upgrade;

class Mob : public Entity {
public:
  float hp = 1, maxHp = 1, flash = 0, animT = 0, facing = 1, scale = 1;
  Status status;
  Traits traits;
  double xAim = 1, yAim = 0;
  float invuln = 0, speed = 74, might = 1, haste = 1, magnet = 28, armor = 0,
        regen = 0, luck = 0, heroTimer = 0, skillTime = 0, knifeCooldown = 0;
  int heroCount = 0;
  std::vector<std::unique_ptr<Upgrade>> upgrades;

  Mob();
  ~Mob() override;
  void tickKit();
  Upgrade *grant(const char *name, int levels);

  virtual bool hostile() const { return false; }
  virtual void split() {}
  virtual float power() const { return might; }
  virtual void aim(double &ax, double &ay) const { ax = xAim, ay = yAim; }
  virtual void strike(float, double, double, float, Upgrade * = nullptr) {}
  virtual class Player *asPlayer() { return nullptr; }
  virtual void mend(float amount) { hp = std::min(maxHp, hp + amount); }
  virtual void shield(float) {}
  virtual Sprite body() const { return {}; }

  bool afflict(StatusKind kind, float duration, float power,
               Upgrade *src = nullptr) {
    return !removed && hp > 0 && status.apply(kind, duration, power, src);
  }

  double bodyY() const { return y - 7 * scale; }
  bool dead() const { return hp <= 0; }
  float occlusion(float &width) const override {
    return width = 0.8f * scale, 14 * scale;
  }

protected:
  void drawBody(Screen &s, int px, int py, Sprite body, bool floats, float move,
                Color tint, float flashAmount, Color flashColor) const;
  void drawShadow(Screen &s, int px, int py, bool floats) const;
};
