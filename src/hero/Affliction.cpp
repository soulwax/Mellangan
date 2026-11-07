#include "hero/Affliction.hpp"

#include "entity/Player.hpp"
#include "hero/Mutation.hpp"
#include "level/Level.hpp"

namespace {

using A = Affliction;

struct Source {
  BiomeId surface;
  CaveBiome below[2];
  Affliction gives;
  bool night;
};

constexpr Source SOURCES[] = {
    {BIOME_FUNGAL, {CaveBiome::Spores, CaveBiome::Glowcap}, A::SporeCrown, false},
    {BIOME_GLASS, {CaveBiome::Geodes, CaveBiome::Cathedral}, A::GlassSkin, false},
    {BIOME_CRYSTAL, {CaveBiome::Geodes, CaveBiome::Cathedral}, A::GlassSkin, false},
    {BIOME_SNOW, {CaveBiome::Rime, CaveBiome::Rime}, A::FrostBlood, false},
    {BIOME_TAIGA, {CaveBiome::Rime, CaveBiome::Rime}, A::FrostBlood, false},
    {BIOME_ASHLANDS, {CaveBiome::Vents, CaveBiome::Magma}, A::AshLung, false},
    {BIOME_EMBER, {CaveBiome::Vents, CaveBiome::Magma}, A::AshLung, false},
    {BIOME_MOON, {CaveBiome::Count, CaveBiome::Count}, A::MoonTouched, true},
};

constexpr float GLASS_ARMOR = 5;

void immunities(Player &p) {
  uint32_t chill = 1u << int(StatusKind::Chill), poison = 1u << int(StatusKind::Poison);
  p.status.immune &= ~(chill | poison);
  if (affliction::has(p, A::FrostBlood))
    p.status.immune |= chill;
  if (affliction::has(p, A::SporeCrown) || mutation::has(p, Mutation::SporeSacs))
    p.status.immune |= poison;
}

} // namespace

const AfflictionDef AFFLICTIONS[int(Affliction::Count)] = {
    {"SPORE CROWN"}, {"GLASS SKIN"}, {"FROST BLOOD"}, {"ASH LUNG"}, {"MOON TOUCHED"},
};

namespace affliction {

bool has(const Player &p, Affliction a) { return p.afflictions >> int(a) & 1; }

void grant(Player &p, Affliction a) {
  if (a == Affliction::Count || has(p, a))
    return;
  p.afflictions |= uint8_t(1u << int(a));
  if (a == A::GlassSkin)
    p.armor += GLASS_ARMOR;
  if (a == A::AshLung)
    p.status.resist[int(StatusKind::Burn)] += 0.5f;
  immunities(p);
}

void cure(Player &p, Affliction a) {
  if (!has(p, a))
    return;
  p.afflictions &= uint8_t(~(1u << int(a)));
  if (a == A::GlassSkin)
    p.armor -= GLASS_ARMOR;
  if (a == A::AshLung)
    p.status.resist[int(StatusKind::Burn)] -= 0.5f;
  immunities(p);
}

void tick(Player &p) {
  bool day = p.level->night.dawn;
  p.pace = (has(p, A::SporeCrown) && day ? 0.9f : 1) * (has(p, A::AshLung) ? 0.95f : 1);
  p.vigor = has(p, A::MoonTouched) ? (day ? 0.9f : 1.15f) : 1;
}

Affliction source(BiomeId surface, bool night) {
  for (const Source &s : SOURCES)
    if (s.surface == surface && (!s.night || night))
      return s.gives;
  return Affliction::Count;
}

Affliction source(CaveBiome below) {
  for (const Source &s : SOURCES)
    if (s.below[0] == below || s.below[1] == below)
      return s.gives;
  return Affliction::Count;
}

float ground(const Player &p, Stuff s) {
  if ((s == Stuff::Charged && has(p, A::GlassSkin)) || (s == Stuff::Burning && has(p, A::FrostBlood)))
    return 2;
  return 1;
}

} // namespace affliction
