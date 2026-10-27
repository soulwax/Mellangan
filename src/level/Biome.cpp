#include "level/Biome.hpp"

#include "level/Noise.h"

namespace f = feature;

// structureWeights: cottage, graveyard, shrine, camp, ruin
const BiomeDef BIOMES[BIOME_COUNT] = {
    {"MEADOW", {0.55f, 0.9f, 0.35f, 1}, {1, 1, 1, 1}, 0.34f, 6, 22, 55, {50, 10, 15, 15, 10},
     {{&f::oak, 4, 110}, {&f::bush, 4, 60}, {&f::pine, 2, 50}, {&f::sprouts, 4, 0},
      {&f::beehive, 2, 0}, {&f::sign, 1, 0}, {&f::well, 1, 0}, {&f::log, 1, 0}}},
    {"FOREST", {0.2f, 0.7f, 0.3f, 1}, {0.88f, 0.96f, 0.86f, 1}, 0.42f, 1, 30, 50, {5, 0, 10, 45, 40},
     {{&f::oak, 30, 330}, {&f::pine, 25, 300}, {&f::bush, 10, 80}, {&f::mushrooms, 6, 0},
      {&f::log, 4, 0}, {&f::sprouts, 3, 0}}},
    {"AUTUMN", {1.0f, 0.55f, 0.2f, 1}, {1.04f, 0.94f, 0.8f, 1}, 0.4f, 1, 28, 50, {20, 0, 10, 35, 35},
     {{&f::autumnOak, 25, 300}, {&f::autumnPine, 20, 260}, {&f::bush, 6, 40}, {&f::mushrooms, 8, 0},
      {&f::log, 5, 0}, {&f::sack, 1, 0}}},
    {"HEATH", {0.9f, 0.82f, 0.45f, 1}, {1.06f, 1.0f, 0.82f, 1}, 0.24f, 0, 35, 45, {10, 0, 15, 30, 45},
     {{&f::bush, 12, 120}, {&f::pine, 3, 40}, {&f::sprouts, 6, 0}, {&f::log, 4, 0},
      {&f::sack, 2, 0}, {&f::crate, 1, 0}, {&f::barrel, 1, 0}, {&f::crystal, 1, 0}}},
    {"BARROW", {0.6f, 0.55f, 1.0f, 1}, {0.8f, 0.86f, 1.0f, 1}, 0.3f, 0, 12, 55, {0, 65, 25, 0, 10},
     {{&f::grave, 8, 90}, {&f::bush, 6, 50}, {&f::autumnPine, 4, 50}, {&f::mushrooms, 6, 0},
      {&f::crystal, 3, 0}}},
};

namespace world {

Climate climateAt(int x, int y) {
    // Warp the sample point a couple of tiles so biome borders come out ragged instead of smooth contours.
    float wx = x + noise_perlin(x * 0.21f + 11.3f, y * 0.21f - 4.1f, 701) * 2.5f;
    float wy = y + noise_perlin(x * 0.21f - 7.7f, y * 0.21f + 9.2f, 702) * 2.5f;
    Climate c;
    c.temp = noise_fbm(wx * 0.011f, wy * 0.011f, 3, 101);
    c.moist = noise_fbm(wx * 0.013f + 40.5f, wy * 0.013f, 3, 202);
    c.eerie = noise_fbm(wx * 0.017f, wy * 0.017f - 60.5f, 2, 303);
    return c;
}

BiomeId biomeOf(Climate c) {
    if (c.eerie > 0.26f) return BIOME_BARROW;
    if (c.moist < -0.18f) return BIOME_HEATH;
    if (c.moist > 0.03f) return c.temp > 0.0f ? BIOME_AUTUMN : BIOME_FOREST;
    return BIOME_MEADOW;
}

// Every visible tile asks for its biome several times a frame (dirt rims, tint, props), so memoise per tile.
BiomeId biomeAt(int x, int y) {
    struct Slot {
        int x, y;
        uint8_t biome;
        bool used;
    };
    static Slot cache[1 << 13];
    Slot *s = &cache[hash2(x, y) & ((1 << 13) - 1)];
    if (!s->used || s->x != x || s->y != y) *s = {x, y, uint8_t(biomeOf(climateAt(x, y))), true};
    return BiomeId(s->biome);
}

float clumpAt(int x, int y) {
    float n = noise_fbm(x * 0.09f + 0.5f, y * 0.09f + 0.5f, 2, 404);
    return clampf((n - 0.05f) * 2.5f, 0, 1);
}

} // namespace world
