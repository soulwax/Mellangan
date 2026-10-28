#pragma once

#include "meta/Codex.hpp"

class Enemy;
class Player;

struct HeroData {
  const char *name, *weapon, *title, *trait, *need;
  bool (*met)(const Codex &);
  int tile;
  float hp, speed, might, haste, magnet, armor, regen, luck;
};

class Hero : public HeroData {
public:
  explicit Hero(const HeroData &data) : HeroData(data) {}
  virtual ~Hero() = default;

  virtual void equip(Player &p) const;
  virtual void tick(Player &) const {}
  virtual float guard(Player &, float damage) const { return damage; }
  virtual float wisdom() const { return 1; }
  virtual void slay(Player &, Enemy &) const {}
};

constexpr int HERO_COUNT = 6;
extern const Hero *const HEROES[HERO_COUNT];

const Hero &heroOf(const Codex &c);
