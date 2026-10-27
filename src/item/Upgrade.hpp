#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Atlas.hpp"

class Player;

class Upgrade {
public:
  static constexpr int MAX_LEVEL = 5;
  static constexpr int SLOTS = 6; // per kind: weapons and passives

  const char *name;
  Sprite icon;
  std::array<const char *, MAX_LEVEL> desc;
  bool weapon, rare = false, evolved = false;
  int level = 0;
  float dealt = 0, power = 1, pace = 1;
  const char *partner = nullptr, *evolvedName = nullptr;

  Upgrade(const char *name, Sprite icon,
          std::array<const char *, MAX_LEVEL> desc, bool weapon)
      : name(name), icon(icon), desc(desc), weapon(weapon) {}
  virtual ~Upgrade() = default;

  bool maxed() const { return level >= (rare ? 1 : MAX_LEVEL); }
  bool canEvolve(const Player &p) const;
  void evolve() { evolved = true, name = evolvedName, power = 2, pace = 0.7f; }
  virtual void levelUp(Player &) { level++; }
  virtual void tick(Player &) {}

  static std::vector<std::unique_ptr<Upgrade>> all();

protected:
  static int at(int lvl, int from) { return lvl >= from ? 1 : 0; }
};
