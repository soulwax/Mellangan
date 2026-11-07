#pragma once

#include <cstdint>

#include "level/Matter.hpp"
#include "level/biome/Biome.hpp"
#include "level/biome/CaveBiome.hpp"

class Player;

enum class Affliction : uint8_t { SporeCrown, GlassSkin, FrostBlood, AshLung, MoonTouched, Count };

struct AfflictionDef {
  const char *name;
};

extern const AfflictionDef AFFLICTIONS[int(Affliction::Count)];

namespace affliction {

constexpr float WARN = 40, CATCH = 75, FADE = 0.5f, CURE_GOLD = 150, CAUGHT_SCRUTINY = 5, SPREAD_SCRUTINY = 0.2f;

bool has(const Player &p, Affliction a);
void grant(Player &p, Affliction a);
void cure(Player &p, Affliction a);
void tick(Player &p);
Affliction source(BiomeId surface, bool night);
Affliction source(CaveBiome below);
float ground(const Player &p, Stuff s);

} // namespace affliction
