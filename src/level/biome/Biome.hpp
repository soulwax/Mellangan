#pragma once

#include "entity/enemy/Enemy.hpp"
#include "level/Feature.hpp"
#include "level/Structure.hpp"

class Level;

enum BiomeId { BIOME_MEADOW, BIOME_FOREST, BIOME_AUTUMN, BIOME_HEATH, BIOME_BARROW, BIOME_SNOW, BIOME_BLOSSOM, BIOME_MIRE, BIOME_DESERT,
              BIOME_JUNGLE, BIOME_SAVANNA, BIOME_ASHLANDS, BIOME_CRYSTAL, BIOME_FUNGAL, BIOME_COUNT };

struct Climate {
    float temp, moist, eerie;
};

struct FeatureSpawn {
    const Feature *feature;
    int base, dense;
};

constexpr uint32_t foe(EnemyKind k) { return 1u << int(k); }

constexpr int MAX_SPAWNS = 20;
struct BiomeData {
    const char *name;
    Color debug, tint;
    float dirt;
    int flowers, tufts;
    int structureChance;
    int structureWeights[int(StructureKind::Count)];
    uint32_t foes;
    FeatureSpawn spawns[MAX_SPAWNS];
    const Feature *fringe;
};

class Biome : public BiomeData {
public:
    explicit Biome(const BiomeData &data) : BiomeData(data) {}
    virtual ~Biome() = default;

    virtual bool winter() const { return false; }
    virtual Sprite grass(int kind) const;
    virtual Sprite dirtRim(int offset) const;
    virtual Sprite frost(Sprite ground) const { return ground; }
    virtual void weather(Level &, double, double) const {}
    virtual bool ground(int, int, Sprite *) const { return false; }
    virtual int place(int, int, uint32_t, Prop &) const { return -1; }
};

extern const Biome *const BIOMES[BIOME_COUNT];

namespace world {

Climate climateAt(int x, int y);
BiomeId biomeOf(Climate c);
BiomeId biomeAt(int x, int y);
float clumpAt(int x, int y);
int rim(int mask);

} // namespace world
