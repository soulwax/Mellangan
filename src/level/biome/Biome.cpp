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
#include "level/biome/ExpansionBiome.hpp"

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
    &CRYSTAL, &FUNGAL, &BADLANDS, &TAIGA,
    expansionBiome(0),expansionBiome(1),expansionBiome(2),expansionBiome(3),
    expansionBiome(4),expansionBiome(5),expansionBiome(6),expansionBiome(7),
    expansionBiome(8),expansionBiome(9)};

namespace world {

Climate climateAt(int x, int y) {
  float sx = x / CLIMATE_SCALE;
  float sy = y / CLIMATE_SCALE;
  float wx = sx + noise_fbm(sx * 0.045f + 11.3f, sy * 0.045f - 4.1f, 2, 701) * 5 +
             noise_perlin(sx * 0.23f + 3.1f, sy * 0.23f - 8.7f, 703) * 0.9f;
  float wy = sy + noise_fbm(sx * 0.045f - 7.7f, sy * 0.045f + 9.2f, 2, 702) * 5 +
             noise_perlin(sx * 0.23f - 5.3f, sy * 0.23f + 2.9f, 704) * 0.9f;
  Climate c;
  c.temp = noise_fbm(wx * 0.011f, wy * 0.011f, 3, 101);
  c.moist = noise_fbm(wx * 0.013f + 40.5f, wy * 0.013f, 3, 202);
  c.eerie = noise_fbm(wx * 0.017f, wy * 0.017f - 60.5f, 2, 303);
  c.wild = noise_fbm(wx * 0.012f + 123.5f, wy * 0.012f - 77.5f, 2, 505);
  return c;
}

BiomeId biomeOf(Climate c) {
  if (c.wild > WILD) {
    int band = c.temp < -0.2f ? 0 : c.temp < -0.07f ? 1 :
               c.temp < 0.07f ? 2 : c.temp < 0.2f ? 3 : 4;
    return BiomeId(BIOME_SALT + band * 2 + (c.moist > 0 ? 1 : 0));
  }
  if (c.eerie > 0.14f) {
    if (c.temp > 0.12f)
      return BIOME_ASHLANDS;
    if (c.temp < -0.12f)
      return BIOME_CRYSTAL;
    return c.moist > 0.06f ? BIOME_FUNGAL : BIOME_BARROW;
  }
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

void siteOf(int x, int y, int &sx, int &sy) {
  float fx = x + noise_fbm(x * 0.018f + 3.7f, y * 0.018f - 1.3f, 2, 611) * 30 +
             noise_perlin(x * 0.09f, y * 0.09f + 5.1f, 612) * 3;
  float fy = y + noise_fbm(x * 0.018f - 8.1f, y * 0.018f + 6.4f, 2, 613) * 30 +
             noise_perlin(x * 0.09f + 2.3f, y * 0.09f, 614) * 3;
  int cx = int(std::floor(fx / BIOME_CELL)), cy = int(std::floor(fy / BIOME_CELL));
  float best = 1e30f;
  for (int j = cy - 1; j <= cy + 1; j++)
    for (int i = cx - 1; i <= cx + 1; i++) {
      uint32_t h = hash2(i * 73 + int(noise_salt), j * 151 - int(noise_salt));
      float px = (i + 0.15f + (h & 255) / 255.0f * 0.7f) * BIOME_CELL,
            py = (j + 0.15f + (h >> 8 & 255) / 255.0f * 0.7f) * BIOME_CELL;
      float d = (px - fx) * (px - fx) + (py - fy) * (py - fy);
      if (d < best)
        best = d, sx = int(px), sy = int(py);
    }
}

BiomeId biomeForLocation(int x, int y) {
  int sx, sy;
  siteOf(x, y, sx, sy);
  Climate c = climateAt(sx, sy);
  double distance = std::hypot(double(sx), double(sy));
  float blend = clampf(float((distance - 260.0) / 400.0), 0, 1);
  blend = blend * blend * (3 - 2 * blend);
  c.eerie *= blend;
  c.wild = lerpf(-1, c.wild, blend);
  return biomeOf(c);
}

float clumpAt(int x, int y) {
  float n = noise_fbm(x * 0.09f + 0.5f, y * 0.09f + 0.5f, 2, 404);
  return clampf((n - 0.05f) * 2.5f, 0, 1);
}

} // namespace world
