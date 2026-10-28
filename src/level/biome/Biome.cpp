#include "level/biome/Biome.hpp"

#include "level/Noise.h"
#include "level/biome/Ashlands.hpp"
#include "level/biome/Autumn.hpp"
#include "level/biome/Badlands.hpp"
#include "level/biome/Barrow.hpp"
#include "level/biome/Blossom.hpp"
#include "level/biome/CrystalFields.hpp"
#include "level/biome/Desert.hpp"
#include "level/biome/Forest.hpp"
#include "level/biome/FungalWood.hpp"
#include "level/biome/Heath.hpp"
#include "level/biome/Jungle.hpp"
#include "level/biome/Meadow.hpp"
#include "level/biome/Mire.hpp"
#include "level/biome/Savanna.hpp"
#include "level/biome/Snow.hpp"
#include "level/biome/Taiga.hpp"

namespace tt = atlas::tt;

Sprite Biome::grass(int kind) const {
  static const int TILES[3] = {tt::Grass, tt::GrassTufts, tt::GrassFlowers};
  return atlas::town(TILES[kind]);
}

Sprite Biome::dirtRim(int offset) const {
  return atlas::town(tt::DirtTopLeft + offset);
}

namespace {
const Meadow MEADOW;
const Forest FOREST;
const Autumn AUTUMN;
const Heath HEATH;
const Barrow BARROW;
const Snow SNOW;
const Blossom BLOSSOM;
const Mire MIRE;
const Desert DESERT;
const Jungle JUNGLE;
const Savanna SAVANNA;
const Ashlands ASHLANDS;
const CrystalFields CRYSTAL;
const FungalWood FUNGAL;
const Badlands BADLANDS;
const Taiga TAIGA;
} // namespace

const Biome *const BIOMES[BIOME_COUNT] = {
    &MEADOW,  &FOREST, &AUTUMN,   &HEATH,  &BARROW,  &SNOW,
    &BLOSSOM, &MIRE,   &DESERT,   &JUNGLE, &SAVANNA, &ASHLANDS,
    &CRYSTAL, &FUNGAL, &BADLANDS, &TAIGA};

namespace world {

Climate climateAt(int x, int y) {
  float wx = x + noise_fbm(x * 0.045f + 11.3f, y * 0.045f - 4.1f, 2, 701) * 5 +
             noise_perlin(x * 0.23f + 3.1f, y * 0.23f - 8.7f, 703) * 0.9f;
  float wy = y + noise_fbm(x * 0.045f - 7.7f, y * 0.045f + 9.2f, 2, 702) * 5 +
             noise_perlin(x * 0.23f - 5.3f, y * 0.23f + 2.9f, 704) * 0.9f;
  Climate c;
  c.temp = noise_fbm(wx * 0.011f, wy * 0.011f, 3, 101);
  c.moist = noise_fbm(wx * 0.013f + 40.5f, wy * 0.013f, 3, 202);
  c.eerie = noise_fbm(wx * 0.017f, wy * 0.017f - 60.5f, 2, 303);
  return c;
}

BiomeId biomeOf(Climate c) {
  if (c.eerie > 0.14f && c.temp > 0.2f)
    return BIOME_ASHLANDS;
  if (c.eerie > 0.21f && c.temp < -0.14f)
    return BIOME_CRYSTAL;
  if (c.eerie > 0.14f && c.moist > 0.18f)
    return BIOME_FUNGAL;
  if (c.eerie > 0.26f)
    return BIOME_BARROW;
  if (c.temp < -0.12f && c.temp > -0.38f && c.moist > 0.08f && c.moist < 0.26f)
    return BIOME_TAIGA;
  if (c.temp < -0.24f)
    return BIOME_SNOW;
  if (c.temp > 0.15f && c.moist > 0.12f)
    return BIOME_JUNGLE;
  if (c.temp > 0.0f && c.temp < 0.3f && c.moist < -0.1f && c.eerie < -0.14f)
    return BIOME_BADLANDS;
  if (c.temp > 0.1f && c.moist < -0.16f)
    return BIOME_DESERT;
  if (c.moist < -0.18f)
    return BIOME_HEATH;
  if (c.temp > 0.17f && c.moist < -0.06f)
    return BIOME_SAVANNA;
  if (c.temp > 0.18f && c.moist <= 0.03f)
    return BIOME_BLOSSOM;
  if (c.moist > 0.24f && c.temp < 0.08f)
    return BIOME_MIRE;
  if (c.moist > 0.03f)
    return c.temp > 0.0f ? BIOME_AUTUMN : BIOME_FOREST;
  return BIOME_MEADOW;
}

float clumpAt(int x, int y) {
  float n = noise_fbm(x * 0.09f + 0.5f, y * 0.09f + 0.5f, 2, 404);
  return clampf((n - 0.05f) * 2.5f, 0, 1);
}

} // namespace world
