#pragma once

#include <string>
#include <string_view>

enum class Stat {
  None,
  Damage,
  Burn,
  Chill,
  Poison,
  Shock,
  Bleed,
  Projectiles,
  Area,
  Crit,
  Leech,
  Armour,
  Life,
  MagicFind,
  GoldFind,
  Speed,
  Haste,
  Regen,
  Reach,
  Wisdom,
  Thorns,
  ResistBurn,
  ResistChill,
  ResistPoison,
  Kindling,
  NightDamage,
  BossDamage,
  EliteDamage,
  Arms,
  Arts,
  Instincts,
  Block,
  ProjectileSpeed,
  Light,
};

namespace slot {
constexpr unsigned Weapon = 1, Offhand = 2, Armour = 4, Charm = 8, AreaWeapon = 16,
                   ShotWeapon = 32, Any = Weapon | Offhand | Armour | Charm;
} // namespace slot

struct AffixTier {
  const char *name;
  int qlvl;
  float lo, hi;
};

struct AffixDef {
  const char *key;
  bool prefix;
  unsigned slots;
  Stat stat;
  bool classOnly;
  int tiers;
  AffixTier tier[5];
};

extern const AffixDef AFFIXES[];
extern const int AFFIX_COUNT;

int findAffix(std::string_view key);
std::string describe(Stat stat, float value);
