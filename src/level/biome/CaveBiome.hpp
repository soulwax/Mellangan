#pragma once

#include <cstdint>

#include "level/Feature.hpp"
#include "level/biome/Biome.hpp"

enum class CaveBiome : uint8_t {
  Roots,
  Grotto,
  Rime,
  Warrens,
  Spores,
  Vents,
  Geodes,
  Catacombs,
  Glowcap,
  Mere,
  Magma,
  Cathedral,
  Ossuary,
  Halls,
  Abyss,
  Hive,
  Count
};

constexpr int CAVE_SHALLOW = int(CaveBiome::Glowcap);

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
  Color grade{0.78f, 0.8f, 0.92f, 1};
  float gloom = 0.45f;
  const char *guardian = nullptr;
  EnemyKind keeper = EnemyKind::Rat;
};

extern const CaveDef CAVES[int(CaveBiome::Count)];
CaveBiome caveBelow(BiomeId above);
CaveBiome deepClimate(float heat, float damp, float age);
