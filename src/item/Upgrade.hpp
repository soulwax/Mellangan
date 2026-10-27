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
  bool weapon;
  int level = 0;

  Upgrade(const char *name, Sprite icon,
          std::array<const char *, MAX_LEVEL> desc, bool weapon)
      : name(name), icon(icon), desc(desc), weapon(weapon) {}
  virtual ~Upgrade() = default;

  bool maxed() const { return level >= MAX_LEVEL; }
  virtual void levelUp(Player &) { level++; }
  virtual void tick(Player &) {}

  static std::vector<std::unique_ptr<Upgrade>> all();

protected:
  static int at(int lvl, int from) { return lvl >= from ? 1 : 0; }
};
