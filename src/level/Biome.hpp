#pragma once

#include "entity/enemy/Enemy.hpp"
#include "level/Feature.hpp"
#include "level/Structure.hpp"

enum BiomeId { BIOME_MEADOW, BIOME_FOREST, BIOME_AUTUMN, BIOME_HEATH, BIOME_BARROW, BIOME_SNOW, BIOME_COUNT };

// Low-frequency Perlin fields the biome is picked from, each roughly [-1, 1].
struct Climate {
    float temp, moist, eerie;
};

// Per-mille chance per open tile: base everywhere in the biome, plus dense scaled by the local clump field.
struct FeatureSpawn {
    const Feature *feature;
    int base, dense;
};

constexpr uint32_t foe(EnemyKind k) { return 1u << int(k); }

constexpr int MAX_SPAWNS = 8;
struct BiomeDef {
    const char *name;
    Color debug, tint;
    float dirt;               // detail noise above this is bare dirt
    int flowers, tufts;       // percent of grass tiles
    int structureChance;      // percent of regions holding a structure
    int structureWeights[int(StructureKind::Count)];
    uint32_t foes;            // foe() bits the spawner favours here once unlocked
    FeatureSpawn spawns[MAX_SPAWNS]; // rolled in order, ends at the first null feature
    bool winter = false;
};

extern const BiomeDef BIOMES[BIOME_COUNT];

namespace world {

Climate climateAt(int x, int y);
BiomeId biomeOf(Climate c);
BiomeId biomeAt(int x, int y);
float clumpAt(int x, int y);

} // namespace world
