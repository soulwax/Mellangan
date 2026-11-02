#include "item/loot/Item.hpp"
#include "item/loot/Unique.hpp"

#include <spdlog/fmt/fmt.h>

#include <sstream>

const Curse CURSES[CURSE_COUNT] = {
    {"Frail", "-10% max health"},
    {"Leaden", "-5% movement speed"},
    {"Hunted", "foes deal 5% more damage to you"},
    {"Bleeding", "lose 1 health per second"},
    {"Dimmed", "-20% light radius"},
};

namespace {

const char *const FIRST[] = {"Dusk",  "Ember",   "Hollow", "Grave",  "Moon",  "Ash",
                             "Storm", "Rune",    "Blood",  "Dread",  "Gloom", "Frost",
                             "Thorn", "Wraith",  "Star",   "Bone",   "Shadow", "Night",
                             "Cinder", "Bramble", "Raven", "Iron",   "Doom",  "Pale"};
const char *const WEAPONS[] = {"Fang", "Bite",  "Song",  "Edge",  "Thorn", "Spike",
                               "Wake", "Call",  "Brand", "Coil",  "Lash",  "Shard",
                               "Gnash", "Howl", "Spire", "Talon"};
const char *const OFFHANDS[] = {"Ward", "Tome", "Eye",  "Lamp",    "Veil",  "Mark",
                                "Sigil", "Quill", "Bulwark", "Aegis", "Lantern", "Grasp"};
const char *const ARMOURS[] = {"Shroud", "Hide", "Mantle", "Shell", "Coat",  "Carapace",
                               "Skin",   "Wrap", "Guard",  "Pelt",  "Cloak", "Plate"};
const char *const CHARMS[] = {"Eye",  "Heart", "Tear",  "Knot", "Seal",  "Charm",
                              "Idol", "Bead",  "Stone", "Token", "Rune", "Mote"};

template <size_t N> const char *pick(const char *const (&words)[N], uint32_t h) {
  return words[h % N];
}

const char *secondWord(unsigned slots, uint32_t h) {
  if (slots & slot::Weapon)
    return pick(WEAPONS, h);
  if (slots & slot::Offhand)
    return pick(OFFHANDS, h);
  if (slots & slot::Armour)
    return pick(ARMOURS, h);
  return pick(CHARMS, h);
}

float rollValue(const AffixTier &t, Rng &rng) {
  float v = lerpf(t.lo, t.hi, rng.unit());
  bool whole = t.lo == std::floor(t.lo) && t.hi == std::floor(t.hi);
  return whole ? std::round(v) : std::round(v * 10) / 10;
}

const AffixTier &tierOf(const Roll &r) { return AFFIXES[r.affix].tier[r.tier]; }

} // namespace

const char *rarityName(Rarity r) {
  constexpr const char *names[] = {"normal", "superior", "magic", "rare",
                                   "unique", "crafted"};
  return names[int(r)];
}

const char *gearName(int slot) {
  constexpr const char *names[GEAR] = {"WEAPON", "OFF-HAND", "ARMOUR", "CHARM", "CHARM"};
  return names[slot];
}

int Item::gearSlot() const {
  unsigned s = def().slots;
  return s & slot::Weapon ? GearWeapon : s & slot::Offhand ? GearOffhand
                                     : s & slot::Armour   ? GearArmour
                                                          : GearCharm;
}

float Item::implicit() const {
  constexpr float SCALE[TIER_COUNT] = {1, 1.6f, 2.3f};
  float v = def().amount * SCALE[tier] * (rarity == Rarity::Superior ? 1.15f : 1);
  return std::round(v * 10) / 10;
}

float Item::stat(Stat s) const {
  float total = def().implicit == s ? implicit() : 0;
  for (int i = 0; i < count; i++)
    if (AFFIXES[rolls[i].affix].stat == s)
      total += rolls[i].value;
  return total;
}

int Item::prefixes() const {
  int n = 0;
  for (int i = 0; i < count; i++)
    n += AFFIXES[rolls[i].affix].prefix;
  return n;
}

int Item::suffixes() const { return count - prefixes(); }

bool Item::hasFamily(int affix) const {
  for (int i = 0; i < count; i++)
    if (rolls[i].affix == affix)
      return true;
  return false;
}

bool Item::addAffix(bool prefix, Rng &rng) {
  if (count >= MAX_AFFIXES)
    return false;
  const ItemBase &b = def();
  int candidates[64][2], n = 0;
  for (int a = 0; a < AFFIX_COUNT; a++) {
    const AffixDef &d = AFFIXES[a];
    if (d.prefix != prefix || !(d.slots & b.slots) || hasFamily(a) ||
        (d.classOnly && !b.klass))
      continue;
    for (int t = 0; t < d.tiers && n < 64; t++)
      if (d.tier[t].qlvl <= ilvl)
        candidates[n][0] = a, candidates[n][1] = t, n++;
  }
  if (!n)
    return false;
  int k = rng.irange(0, n - 1);
  Roll &r = rolls[count++];
  r.affix = candidates[k][0], r.tier = candidates[k][1];
  r.value = rollValue(tierOf(r), rng);
  return true;
}

void Item::reroll(int i, Rng &rng) { rolls[i].value = rollValue(tierOf(rolls[i]), rng); }

std::string Item::baseName() const {
  return fmt::format("{} {}", TIER_NAMES[tier], def().name);
}

const char *Item::grants() const {
  if (const UniqueDef *u = uniqueOf(*this))
    return u->grants ? u->grants : def().grants;
  return def().grants;
}

std::string Item::name() const {
  if (const UniqueDef *u = uniqueOf(*this))
    return u->name;
  std::string out;
  if (rarity == Rarity::Rare || rarity == Rarity::Crafted) {
    uint32_t h = hash2(int(seed), 17), k = hash2(int(seed), 91);
    out = fmt::format("{} {}", pick(FIRST, h), secondWord(def().slots, k));
  } else if (rarity == Rarity::Magic) {
    const char *pre = nullptr, *suf = nullptr;
    for (int i = 0; i < count; i++)
      (AFFIXES[rolls[i].affix].prefix ? pre : suf) = tierOf(rolls[i]).name;
    out = baseName();
    if (pre)
      out = fmt::format("{} {}", pre, out);
    if (suf)
      out = fmt::format("{} {}", out, suf);
  } else {
    out = rarity == Rarity::Superior ? "Superior " + baseName() : baseName();
  }
  return night() ? "Night-touched " + out : out;
}

Color Item::color() const {
  if (night())
    return {0.72f, 0.45f, 1, 1};
  switch (rarity) {
  case Rarity::Superior:
    return {0.72f, 0.72f, 0.74f, 1};
  case Rarity::Magic:
    return {0.45f, 0.55f, 1, 1};
  case Rarity::Rare:
    return {1, 0.92f, 0.35f, 1};
  case Rarity::Unique:
    return {0.82f, 0.68f, 0.38f, 1};
  case Rarity::Crafted:
    return {1, 0.62f, 0.2f, 1};
  default:
    return {1, 1, 1, 1};
  }
}

int Item::value() const {
  constexpr float RARITY[] = {1, 1.4f, 2.5f, 5, 12, 6};
  return int(def().value * (1 + ilvl / 10.0f) * (1 + tier * 0.5f) * RARITY[int(rarity)]);
}

std::string Item::save() const {
  std::string out = fmt::format("{} {} {} {} {} {}", def().key, tier, ilvl, int(rarity),
                                seed, curse);
  for (int i = 0; i < count; i++) {
    std::string key = AFFIXES[rolls[i].affix].key;
    for (char &c : key)
      if (c == ' ')
        c = '_';
    out += fmt::format(" {}:{}:{}", key, rolls[i].tier, rolls[i].value);
  }
  return out;
}

bool Item::load(std::string_view line, Item &out) {
  std::istringstream in{std::string(line)};
  std::string key;
  int rarity = 0;
  Item item;
  if (!(in >> key >> item.tier >> item.ilvl >> rarity >> item.seed >> item.curse))
    return false;
  item.base = findBase(key);
  if (item.base < 0 || item.tier < 0 || item.tier >= TIER_COUNT || rarity < 0 ||
      rarity >= int(Rarity::Count) || item.curse >= CURSE_COUNT)
    return false;
  item.rarity = Rarity(rarity);
  std::string token;
  while (in >> token && item.count < MAX_AFFIXES) {
    size_t a = token.rfind(':'), b = token.rfind(':', a - 1);
    if (a == std::string::npos || b == std::string::npos || b == 0)
      return false;
    std::string affix = token.substr(0, b);
    for (char &c : affix)
      if (c == '_')
        c = ' ';
    Roll &r = item.rolls[item.count];
    r.affix = findAffix(affix);
    r.tier = std::atoi(token.c_str() + b + 1);
    r.value = float(std::atof(token.c_str() + a + 1));
    if (r.affix < 0 || r.tier < 0 || r.tier >= AFFIXES[r.affix].tiers)
      return false;
    item.count++;
  }
  out = item;
  return true;
}
