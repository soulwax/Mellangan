#pragma once

#include "entity/Entity.hpp"
#include "entity/Status.hpp"
#include "entity/Trait.hpp"

class Mob : public Entity {
public:
  float hp = 1, maxHp = 1, flash = 0, animT = 0, facing = 1, scale = 1;
  Status status;
  Traits traits;

  virtual bool hostile() const { return false; }
  virtual void split() {}
  virtual float power() const { return 1; }
  virtual void aim(double &ax, double &ay) const { ax = facing, ay = 0; }
  virtual void strike(float, double, double, float) {}
  virtual void mend(float amount) { hp = std::min(maxHp, hp + amount); }
  virtual Sprite body() const { return {}; }

  bool afflict(StatusKind kind, float duration, float power,
               Upgrade *src = nullptr) {
    return !removed && hp > 0 && status.apply(kind, duration, power, src);
  }

  double bodyY() const { return y - 7 * scale; }
  float occlusion(float &width) const override {
    return width = 0.8f * scale, 14 * scale;
  }

protected:
  void drawBody(Screen &s, int px, int py, Sprite body, bool floats, float move,
                Color tint, float flashAmount, Color flashColor) const;
  void drawShadow(Screen &s, int px, int py, bool floats) const;
};
