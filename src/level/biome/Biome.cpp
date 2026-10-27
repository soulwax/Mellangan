#include "level/biome/Biome.hpp"

#include "level/biome/Meadow.hpp"
#include "level/biome/Forest.hpp"
#include "level/biome/Autumn.hpp"
#include "level/biome/Heath.hpp"
#include "level/biome/Barrow.hpp"
#include "level/biome/Snow.hpp"
#include "level/biome/Blossom.hpp"
#include "level/biome/Mire.hpp"
#include "level/Noise.h"

namespace tt = atlas::tt;

Sprite Biome::grass(int kind) const {
    static const int TILES[3] = {tt::Grass, tt::GrassTufts, tt::GrassFlowers};
    return atlas::town(TILES[kind]);
}

Sprite Biome::dirtRim(int offset) const { return atlas::town(tt::DirtTopLeft + offset); }

namespace {
const Meadow MEADOW;
const Forest FOREST;
const Autumn AUTUMN;
const Heath HEATH;
const Barrow BARROW;
const Snow SNOW;
const Blossom BLOSSOM;
const Mire MIRE;
} // namespace

const Biome *const BIOMES[BIOME_COUNT] = {&MEADOW, &FOREST, &AUTUMN, &HEATH, &BARROW, &SNOW, &BLOSSOM, &MIRE};

namespace world {

Climate climateAt(int x, int y) {
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
    if (c.temp < -0.24f) return BIOME_SNOW;
    if (c.moist < -0.18f) return BIOME_HEATH;
    if (c.temp > 0.18f && c.moist <= 0.03f) return BIOME_BLOSSOM;
    if (c.moist > 0.24f && c.temp < 0.08f) return BIOME_MIRE;
    if (c.moist > 0.03f) return c.temp > 0.0f ? BIOME_AUTUMN : BIOME_FOREST;
    return BIOME_MEADOW;
}

float clumpAt(int x, int y) {
    float n = noise_fbm(x * 0.09f + 0.5f, y * 0.09f + 0.5f, 2, 404);
    return clampf((n - 0.05f) * 2.5f, 0, 1);
}

} // namespace world
