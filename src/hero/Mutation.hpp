#pragma once

#include <cstdint>

#include "level/biome/Biome.hpp"
#include "level/biome/CaveBiome.hpp"

class Player;

enum class Mutation : uint8_t {
  Chitin,
  HollowBones,
  Quills,
  EmberGlands,
  RimeGlands,
  Gills,
  Sunskin,
  SporeSacs,
  Kindle,
  Stillness,
  Stormcall,
  Brittle,
  Sporebloom,
  Frail,
  Sluggish,
  Count
};

struct MutationDef {
  const char *name;
  bool mind, defect;
  float every;
};

extern const MutationDef MUTATIONS[int(Mutation::Count)];

namespace mutation {

constexpr float SMOTHER = 56, WITNESS = 300, LINGER = 8, WHISPER_COOL = 120, OFFER_LIFE = 12, SURGE_LIFE = 15,
                WALK_AWAY = 160, UPHEAVAL_ODDS = 0.25f, FORCED_SCRUTINY = 10, CAST_SCRUTINY = 2;

bool has(const Player &p, Mutation m);
void grant(Player &p, Mutation m);
void tick(Player &p);
Mutation whisper(BiomeId biome);
void surge(CaveBiome biome, Mutation &a, Mutation &b);
Mutation random(uint32_t roll, bool defect, uint32_t owned);
uint32_t starting(uint32_t seed);

} // namespace mutation
