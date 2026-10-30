#pragma once

#include "entity/projectile/Projectile.hpp"

class Player;

enum class SlashStyle { Moon, Crusader, Flurry };

class Slash : public Projectile {
public:
  Slash(Player &p, float side, float damage, float scale,
        SlashStyle style = SlashStyle::Moon);
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

protected:
  bool touches(const Enemy &e) const override;
  void push(const Enemy &, double &xd, double &yd) const override {
    xd = side, yd = 0;
  }

private:
  Player &player;
  float side, scale;
  SlashStyle style;
  bool emitted = false;
  Color tone() const;
  void emit();
};
