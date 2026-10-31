#pragma once

#include "gfx/Screen.hpp"
#include "meta/Codex.hpp"

class Enemy;
class Mob;
class Player;

struct HeroData {
  const char *name, *weapon, *title, *trait, *need;
  bool (*met)(const Codex &);
  int tile;
  Color tint;
  float hp, speed, might, haste, magnet, armor, regen, luck;
  const char *skill, *skillDesc;
  float cooldown;
  const char *affinity[3];
};

class Hero : public HeroData {
public:
  explicit Hero(const HeroData &data) : HeroData(data) {}
  virtual ~Hero() = default;

  virtual void equip(Mob &p) const;
  virtual void cast(Mob &) const {}
  virtual void tick(Mob &) const {}
  virtual float guard(Mob &, float damage) const { return damage; }
  virtual float wisdom() const { return 1; }
  virtual float devotion() const { return 1; }
  virtual float edge() const { return 1.8f; }
  virtual Sprite sprite() const { return atlas::tile(tile); }
  virtual void slay(Mob &, Enemy &) const {}

  bool favours(const char *upgrade) const;
};

constexpr int HERO_COUNT = 12;
extern const Hero *const HEROES[HERO_COUNT];

const Hero &heroOf(const Codex &c);
