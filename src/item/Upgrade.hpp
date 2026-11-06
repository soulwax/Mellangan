#pragma once

#include <array>
#include <memory>
#include <span>
#include <vector>

#include "Atlas.hpp"

class Enemy;
class Mob;
class Player;

struct Figure {
  enum Kind { Damage, Every, Lasts, Count, Percent };
  const char *name;
  float value;
  Kind kind;
};

struct Figures {
  std::array<Figure, 4> line{};
  int n = 0;
  Figures &add(const char *name, float value, Figure::Kind kind = Figure::Count) {
    if (n < int(line.size()))
      line[n++] = {name, value, kind};
    return *this;
  }
  Figures &damage(float value) { return add("damage", value, Figure::Damage); }
  Figures &every(float value) { return add("every", value, Figure::Every); }
};

class Upgrade {
public:
  static constexpr int MAX_LEVEL = 5;
  static constexpr int SLOTS = 6; // per kind: weapons and passives
  static constexpr float EVOLVED_POWER = 2, EVOLVED_PACE = 0.7f;

  const char *name;
  Sprite icon;
  std::array<const char *, MAX_LEVEL> desc;
  bool weapon, rare = false, evolved = false, skill = false, signature = false;
  int level = 0;
  float dealt = 0, power = 1, pace = 1;
  const char *partner = nullptr, *evolvedName = nullptr, *owner = nullptr;
  Mob *holder = nullptr;
  bool foe = false, hidden = false;
  bool hostile() const { return foe; }

  Upgrade(const char *name, Sprite icon,
          std::array<const char *, MAX_LEVEL> desc, bool weapon)
      : name(name), icon(icon), desc(desc), weapon(weapon) {}
  virtual ~Upgrade() = default;

  bool maxed() const { return level >= (rare ? 1 : MAX_LEVEL); }
  bool canEvolve(const Mob &p) const;
  void evolve() { evolved = true, name = evolvedName, power = EVOLVED_POWER, pace = EVOLVED_PACE; }
  virtual void levelUp(Mob &) { level++; }
  virtual void tick(Mob &) {}
  virtual void slay(Mob &, Enemy &) {}
  virtual float guard(Mob &, float damage) { return damage; }
  virtual void cast(Mob &) {}
  virtual void forget(Mob &) {}
  virtual Figures figures(int, bool) const { return {}; }

  static std::vector<std::unique_ptr<Upgrade>> all();
  static int at(int lvl, int from) { return lvl >= from ? 1 : 0; }
};

std::span<const char *const[3]> evolutionTable();
