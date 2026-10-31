#pragma once

#include <cstdint>

#include "gfx/Screen.hpp"

class Mob;

enum class Trait { Hasted, Vampiric, Warded, Splitting, Volatile, Frosted, Count };

struct TraitDef {
  const char *name;
  Color color;
};
extern const TraitDef TRAITS[int(Trait::Count)];

struct Traits {
  uint32_t bits = 0;
  int ward = 0;
  float trail = 0;

  bool has(Trait t) const { return bits >> int(t) & 1; }
  bool any() const { return bits != 0; }
  void add(Trait t);
  Color tone() const;
  float speed() const { return has(Trait::Hasted) ? 1.4f : 1; }
  float incoming(Mob &self, float damage);
  void landed(Mob &self, float dealt);
  void tick(Mob &self);
  void died(Mob &self, float damage);
  void render(Screen &s, const Mob &self, float radius, float age) const;
};
