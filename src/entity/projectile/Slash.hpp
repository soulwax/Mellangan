#pragma once

#include "entity/projectile/Projectile.hpp"

class Mob;

enum class SlashStyle { Moon, Crusader, Flurry };

class Slash : public Projectile {
public:
  Slash(const Mob &owner, float side, float damage, float scale,
        SlashStyle style = SlashStyle::Moon);
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

protected:
  bool touches(const Mob &e) const override;
  void push(const Mob &, double &xd, double &yd) const override {
    xd = side, yd = 0;
  }

private:
  entt::entity owner;
  double cx, cy, foot;
  float side, scale;
  SlashStyle style;
  bool emitted = false;
  Color tone() const;
  void emit();
};
