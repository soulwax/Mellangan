#include "item/loot/Affix.hpp"

#include <spdlog/fmt/fmt.h>

#include <iterator>

namespace {

using namespace slot;
using S = Stat;

constexpr unsigned WEAPON_OFF = Weapon | Offhand, WEAR = Armour | Charm;

} // namespace

const AffixDef AFFIXES[] = {
    {"damage", true, Weapon, S::Damage, false, 5,
     {{"Sharp", 1, 6, 10}, {"Fine", 12, 12, 18}, {"Cruel", 24, 20, 30},
      {"Savage", 36, 32, 48}, {"Merciless", 48, 50, 70}}},
    {"burn", true, WEAPON_OFF, S::Burn, false, 5,
     {{"Smoldering", 1, 8, 12}, {"Fiery", 12, 13, 19}, {"Searing", 24, 20, 26},
      {"Blazing", 36, 27, 34}, {"Infernal", 48, 35, 45}}},
    {"chill", true, WEAPON_OFF, S::Chill, false, 5,
     {{"Chilling", 1, 8, 12}, {"Frozen", 12, 13, 19}, {"Glacial", 24, 20, 26},
      {"Rime", 36, 27, 34}, {"Arctic", 48, 35, 45}}},
    {"poison", true, WEAPON_OFF, S::Poison, false, 5,
     {{"Noxious", 1, 8, 12}, {"Venomous", 12, 13, 19}, {"Toxic", 24, 20, 26},
      {"Blighted", 36, 27, 34}, {"Pestilent", 48, 35, 45}}},
    {"shock", true, Weapon, S::Shock, false, 5,
     {{"Charged", 1, 6, 10}, {"Shocking", 12, 11, 15}, {"Arcing", 24, 16, 20},
      {"Storming", 36, 21, 25}, {"Thunderous", 48, 26, 32}}},
    {"bleed", true, Weapon, S::Bleed, false, 5,
     {{"Jagged", 1, 8, 12}, {"Serrated", 12, 13, 19}, {"Rending", 24, 20, 26},
      {"Flaying", 36, 27, 34}, {"Butchering", 48, 35, 45}}},
    {"count", true, ShotWeapon, S::Projectiles, false, 3,
     {{"Twin", 12, 1, 1}, {"Triple", 30, 1, 1}, {"Legion", 48, 2, 2}}},
    {"area", true, AreaWeapon | Offhand, S::Area, false, 5,
     {{"Wide", 1, 6, 10}, {"Broad", 12, 11, 17}, {"Vast", 24, 18, 22},
      {"Colossal", 36, 24, 32}, {"Titanic", 48, 33, 40}}},
    {"crit", true, Weapon | Charm, S::Crit, false, 5,
     {{"Keen", 1, 1, 2}, {"Precise", 12, 3, 4}, {"Deadly", 24, 5, 6},
      {"Lethal", 36, 7, 8}, {"Fatal", 48, 9, 11}}},
    {"leech", true, Weapon | Charm, S::Leech, false, 5,
     {{"Thirsty", 1, 1, 1}, {"Hungry", 12, 2, 2}, {"Bloodthirsty", 24, 3, 3},
      {"Vampiric", 36, 4, 4}, {"Sanguine", 48, 5, 5}}},
    {"armour", true, Armour | Offhand | Charm, S::Armour, false, 5,
     {{"Sturdy", 1, 1, 1}, {"Strong", 12, 2, 2}, {"Stalwart", 24, 3, 3},
      {"Bulwark", 36, 4, 5}, {"Impregnable", 48, 6, 8}}},
    {"life", true, WEAR, S::Life, false, 5,
     {{"Hale", 1, 8, 12}, {"Vital", 12, 16, 24}, {"Robust", 24, 30, 40},
      {"Mighty", 36, 45, 55}, {"Titan's", 48, 60, 80}}},
    {"fortune", true, Any, S::MagicFind, false, 5,
     {{"Lucky", 1, 6, 10}, {"Charmed", 12, 12, 18}, {"Blessed", 24, 20, 30},
      {"Fated", 36, 30, 40}, {"Divine", 48, 42, 58}}},
    {"gleaming", true, Any, S::GoldFind, false, 3,
     {{"Gleaming", 1, 8, 12}, {"Glittering", 24, 16, 24}, {"Radiant", 48, 26, 34}}},

    {"speed", false, WEAR, S::Speed, false, 5,
     {{"of the Fox", 1, 2, 4}, {"of the Hare", 12, 4, 6}, {"of the Stag", 24, 7, 9},
      {"of the Wind", 36, 10, 12}, {"of the Gale", 48, 13, 17}}},
    {"haste", false, WEAPON_OFF | Charm, S::Haste, false, 5,
     {{"of Haste", 1, 3, 5}, {"of Celerity", 12, 6, 8}, {"of Alacrity", 24, 9, 11},
      {"of Swiftness", 36, 12, 15}, {"of Quickening", 48, 16, 20}}},
    {"regen", false, WEAR, S::Regen, false, 5,
     {{"of Mending", 1, 0.2f, 0.2f}, {"of Healing", 12, 0.3f, 0.5f},
      {"of Renewal", 24, 0.6f, 0.8f}, {"of Restoration", 36, 0.9f, 1.1f},
      {"of Life Eternal", 48, 1.3f, 1.7f}}},
    {"reach", false, Any, S::Reach, false, 3,
     {{"of the Magnet", 1, 12, 18}, {"of the Lodestone", 24, 25, 35},
      {"of Attraction", 48, 45, 55}}},
    {"wisdom", false, Charm | Offhand, S::Wisdom, false, 4,
     {{"of the Pupil", 1, 3, 5}, {"of the Scholar", 16, 6, 8}, {"of the Sage", 32, 9, 11},
      {"of the Archmage", 48, 13, 17}}},
    {"thorns", false, Armour, S::Thorns, false, 4,
     {{"of Thorns", 1, 4, 6}, {"of Spikes", 16, 8, 12}, {"of Brambles", 32, 15, 21},
      {"of Razors", 48, 26, 34}}},
    {"resist burn", false, WEAR, S::ResistBurn, false, 3,
     {{"of Embers", 1, 15, 25}, {"of the Hearth", 24, 30, 40}, {"of Fire Warding", 48, 45, 55}}},
    {"resist chill", false, WEAR, S::ResistChill, false, 3,
     {{"of Thaw", 1, 15, 25}, {"of Warmth", 24, 30, 40}, {"of Frost Warding", 48, 45, 55}}},
    {"resist poison", false, WEAR, S::ResistPoison, false, 3,
     {{"of Antidote", 1, 15, 25}, {"of Purity", 24, 30, 40}, {"of Venom Warding", 48, 45, 55}}},
    {"kindling", false, Charm, S::Kindling, false, 3,
     {{"of Kindling", 1, 8, 12}, {"of the Torch", 24, 17, 23}, {"of the Beacon", 48, 26, 34}}},
    {"night", false, Weapon | Charm, S::NightDamage, false, 3,
     {{"of Dusk", 1, 4, 6}, {"of Midnight", 24, 8, 12}, {"of the Long Night", 48, 13, 17}}},
    {"dawn", false, Weapon, S::BossDamage, false, 3,
     {{"of Daybreak", 1, 5, 7}, {"of the Dawn", 24, 10, 14}, {"of the Sun", 48, 17, 23}}},
    {"slaying", false, Weapon, S::EliteDamage, false, 3,
     {{"of Slaying", 1, 6, 10}, {"of the Hunt", 24, 12, 16}, {"of the Executioner", 48, 19, 25}}},
    {"arms", false, WEAPON_OFF, S::Arms, true, 2,
     {{"of the Armsmaster", 20, 1, 1}, {"of the Warlord", 45, 2, 2}}},
    {"arts", false, WEAPON_OFF, S::Arts, true, 2,
     {{"of the Adept", 20, 1, 1}, {"of the Master", 45, 2, 2}}},
    {"instincts", false, WEAPON_OFF, S::Instincts, true, 2,
     {{"of the Wild", 20, 1, 1}, {"of the Primal", 45, 2, 2}}},
};

const int AFFIX_COUNT = int(std::size(AFFIXES));

int findAffix(std::string_view key) {
  for (int i = 0; i < AFFIX_COUNT; i++)
    if (key == AFFIXES[i].key)
      return i;
  return -1;
}

std::string describe(Stat stat, float value) {
  switch (stat) {
  case S::Damage:
    return fmt::format("+{:.0f}% weapon damage", value);
  case S::Burn:
    return fmt::format("{:.0f}% chance to burn", value);
  case S::Chill:
    return fmt::format("{:.0f}% chance to chill", value);
  case S::Poison:
    return fmt::format("{:.0f}% chance to poison", value);
  case S::Shock:
    return fmt::format("{:.0f}% chance to chain lightning", value);
  case S::Bleed:
    return fmt::format("{:.0f}% chance to bleed", value);
  case S::Projectiles:
    return fmt::format("+{:.0f} projectile{}", value, value > 1 ? "s" : "");
  case S::Area:
    return fmt::format("+{:.0f}% area", value);
  case S::Crit:
    return fmt::format("+{:.0f}% critical hit chance", value);
  case S::Leech:
    return fmt::format("{:.0f}% of damage healed", value);
  case S::Armour:
    return fmt::format("+{:.0f} armour", value);
  case S::Life:
    return fmt::format("+{:.0f} max health", value);
  case S::MagicFind:
    return fmt::format("+{:.0f}% magic find", value);
  case S::GoldFind:
    return fmt::format("+{:.0f}% gem and gold value", value);
  case S::Speed:
    return fmt::format("+{:.0f}% movement speed", value);
  case S::Haste:
    return fmt::format("-{:.0f}% weapon cooldowns", value);
  case S::Regen:
    return fmt::format("+{:.1f} health per second", value);
  case S::Reach:
    return fmt::format("+{:.0f}% pickup range", value);
  case S::Wisdom:
    return fmt::format("+{:.0f}% experience", value);
  case S::Thorns:
    return fmt::format("{:.0f} damage to melee attackers", value);
  case S::ResistBurn:
    return fmt::format("-{:.0f}% burn on you", value);
  case S::ResistChill:
    return fmt::format("-{:.0f}% chill on you", value);
  case S::ResistPoison:
    return fmt::format("-{:.0f}% poison on you", value);
  case S::Kindling:
    return fmt::format("shrines kindle {:.0f}% faster", value);
  case S::NightDamage:
    return fmt::format("+{:.0f}% damage at night", value);
  case S::BossDamage:
    return fmt::format("+{:.0f}% damage to bosses", value);
  case S::EliteDamage:
    return fmt::format("+{:.0f}% damage to elites", value);
  case S::Arms:
    return fmt::format("+{:.0f} to Arms", value);
  case S::Arts:
    return fmt::format("+{:.0f} to Arts", value);
  case S::Instincts:
    return fmt::format("+{:.0f} to Instincts", value);
  case S::Block:
    return fmt::format("{:.0f}% chance to block", value);
  case S::ProjectileSpeed:
    return fmt::format("+{:.0f}% projectile speed", value);
  case S::Light:
    return fmt::format("+{:.0f}% light radius", value);
  default:
    return "";
  }
}
