#pragma once

#include <cstdint>

#include "level/Feature.hpp"
#include "level/biome/Biome.hpp"

enum class CaveBiome : uint8_t { Roots, Grotto, Rime, Warrens, Spores, Vents, Geodes, Catacombs, Count };

enum class CavePool : uint8_t { Water, Ice, Lava };

struct CaveSpawn {
  const Feature *feature;
  int per;
};

struct CaveDef {
  const char *name;
  Sprite floor[3];
  int wall;
  Color floorTint, wallTint, debug;
  CaveSpawn spawns[3];
  FoeMask foes;
  float pools;
  CavePool pool;
};

extern const CaveDef CAVES[int(CaveBiome::Count)];
CaveBiome caveBelow(BiomeId above);
