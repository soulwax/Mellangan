#include "item/loot/ItemBase.hpp"

#include <cstring>

#include "item/Upgrade.hpp"
#include "item/weapon/ExpansionWeapons.hpp"
#include "item/weapon/Recipe.hpp"
#include "item/weapon/form/FormBehavior.hpp"

const char *const TIER_NAMES[TIER_COUNT] = {"Worn", "Fine", "Masterwork"};

namespace {

namespace t = atlas::t;
namespace tt = atlas::tt;
namespace kt = atlas::kt;
using S = Stat;

struct Gear {
  const char *name;
  unsigned slots;
  Sprite icon;
  int qlvl, weight, value;
  Stat implicit;
  float amount;
};

const Gear GEAR[] = {
    {"Shield", slot::Offhand, atlas::tile(t::Shield), 1, 10, 30, S::Block, 10},
    {"Tome", slot::Offhand, atlas::tile(128), 4, 8, 40, S::Haste, 6},
    {"Quiver", slot::Offhand, atlas::kit(kt::Fern), 4, 8, 40, S::ProjectileSpeed, 12},
    {"Focus", slot::Offhand, atlas::kit(kt::Amethyst), 8, 7, 45, S::Area, 8},
    {"Lantern", slot::Offhand, atlas::town(tt::Sign), 8, 7, 45, S::Light, 25},
    {"Cloth", slot::Armour, atlas::kit(kt::Sage), 1, 10, 20, S::Speed, 3},
    {"Leather", slot::Armour, atlas::kit(kt::Driftwood), 1, 10, 30, S::Armour, 2},
    {"Mail", slot::Armour, atlas::tile(74), 8, 9, 45, S::Armour, 4},
    {"Plate", slot::Armour, atlas::kit(kt::Obsidian), 16, 7, 60, S::Armour, 6},
    {"Robe", slot::Armour, atlas::kit(kt::Lavender), 6, 8, 40, S::Haste, 10},
    {"Stone Charm", slot::Charm, atlas::kit(kt::Pebbles), 1, 10, 25, S::None, 0},
    {"Bone Charm", slot::Charm, atlas::kit(kt::Skull), 8, 9, 35, S::None, 0},
    {"Glass Charm", slot::Charm, atlas::kit(kt::IceChunk), 16, 8, 45, S::None, 0},
    {"Amber Charm", slot::Charm, atlas::kit(kt::Glowshrooms), 24, 7, 55, S::None, 0},
};

const char *const AREA_WEAPONS[] = {"Frost Nova", "Radiance", "Static Field",
                                    "Warding Halo", "Censer", "Ground Sigil", "Mortar"};

std::string keyOf(const char *name) {
  std::string key = name;
  for (char &c : key)
    if (c == ' ')
      c = '_';
  return key;
}

int recipeIndex(const char *name) {
  for (size_t i = 0; i < RECIPES.size(); i++)
    if (std::strcmp(RECIPES[i].name, name) == 0)
      return int(i);
  return -1;
}

bool areaWeapon(const Upgrade &u, int recipe) {
  if (recipe >= 0)
    return FormBehavior::of(RECIPES[recipe].form).area();
  for (const char *n : AREA_WEAPONS)
    if (std::strcmp(n, u.name) == 0)
      return true;
  return false;
}

ItemBase weaponBase(const Upgrade &u) {
  int recipe = recipeIndex(u.name);
  bool expansion = dynamic_cast<const ExpansionWeapon *>(&u) != nullptr;
  int qlvl = recipe >= 0 ? 8 + recipe % 6 * 6 : expansion ? 10 : u.owner ? 6 : 1;
  ItemBase b{keyOf(u.name), u.name,
             slot::Weapon | (areaWeapon(u, recipe) ? slot::AreaWeapon : slot::ShotWeapon),
             u.icon, qlvl, u.owner ? 6 : 10, 50 + qlvl * 3};
  b.grants = u.name, b.klass = u.owner;
  return b;
}

std::vector<ItemBase> build() {
  std::vector<ItemBase> out;
  for (auto &u : Upgrade::all())
    if (u->weapon && !u->skill && !u->rare)
      out.push_back(weaponBase(*u));
  for (const Gear &g : GEAR) {
    ItemBase b{keyOf(g.name), g.name, g.slots, g.icon, g.qlvl, g.weight, g.value};
    b.implicit = g.implicit, b.amount = g.amount;
    out.push_back(b);
  }
  return out;
}

} // namespace

const std::vector<ItemBase> &itemBases() {
  static const std::vector<ItemBase> bases = build();
  return bases;
}

int findBase(std::string_view key) {
  const auto &bases = itemBases();
  for (size_t i = 0; i < bases.size(); i++)
    if (bases[i].key == key)
      return int(i);
  return -1;
}
