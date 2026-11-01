#pragma once

#include <cmath>
#include <cstdint>

#include "gfx/Screen.hpp"

class Mob;

enum class Trait {
  Hasted,
  Vampiric,
  Warded,
  Splitting,
  Volatile,
  Frosted,
  Thorned,
  Regenerating,
  Blinking,
  Phasing,
  Brooding,
  Gravitic,
  Cursed,
  Silencing,
  FireEnchanted,
  ColdEnchanted,
  StormEnchanted,
  StoneSkin,
  Multishot,
  AuraBearer,
  Count
};

struct TraitDef {
  const char *name;
  Color color;
};
extern const TraitDef TRAITS[int(Trait::Count)];

struct Traits {
  uint32_t bits = 0;
  int ward = 0;
  float trail = 0, calm = 0, blink = 0, thorn = 0, brood = 6, cycle = 0, aura = 0;

  bool has(Trait t) const { return bits >> int(t) & 1; }
  bool any() const { return bits != 0; }
  void add(Trait t);
  Color tone() const;
  float speed() const {
    return (has(Trait::Hasted) ? 1.4f : 1) * (has(Trait::StoneSkin) ? 0.85f : 1);
  }
  bool veiled() const { return has(Trait::Phasing) && std::fmod(cycle, 4.0f) > 3; }
  float incoming(Mob &self, float damage);
  void landed(Mob &self, float dealt);
  void tick(Mob &self);
  void died(Mob &self, float damage);
  void render(Screen &s, const Mob &self, float radius, float age) const;
};
