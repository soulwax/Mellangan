#pragma once

#include "level/World.hpp"
#include "level/Noise.h"

namespace world {

struct TileMemo {
  int32_t x, y;
  uint32_t bits, epoch;
  float shade, clump;
};

struct MemoField {
  uint32_t has;
  int shift;
  uint32_t mask;
};

namespace slot {
constexpr MemoField biome{1 << 5, 0, 31}, dirt{1 << 6, 7, 1},
    raw{1 << 10, 13, 31}, rawDirt{1 << 11, 12, 1}, rawWater{1 << 18, 19, 1},
    water{1 << 20, 21, 1}, level{1 << 22, 23, 3}, rawLevel{1 << 25, 26, 3};
constexpr uint32_t shade = 1 << 8, clump = 1 << 9;
} // namespace slot

TileMemo *memoAt(int x, int y);

template <class F> uint32_t memo(int x, int y, MemoField f, F compute) {
  TileMemo *m = memoAt(x, y);
  if (!(m->bits & f.has)) {
    uint32_t v = uint32_t(compute());
    m = memoAt(x, y);
    m->bits |= f.has | v << f.shift;
  }
  return m->bits >> f.shift & f.mask;
}

template <class F>
float memo(int x, int y, uint32_t has, float TileMemo::*field, F compute) {
  TileMemo *m = memoAt(x, y);
  if (!(m->bits & has)) {
    float v = compute();
    m = memoAt(x, y);
    m->bits |= has, m->*field = v;
  }
  return m->*field;
}

extern Layer active;
bool mouthProp(int x, int y, Prop &p);

namespace under {
BiomeId biomeAt(int x, int y);
bool pool(int x, int y);
GroundBlend groundBlend(int x, int y);
int heightAt(int x, int y);
bool isDirt(int x, int y);
bool isWater(int x, int y);
bool isRamp(int x, int y);
bool structureIn(int rx, int ry, Structure &out);
Sprite groundTile(int x, int y);
Color groundTint(int x, int y);
bool propAt(int x, int y, Prop &p);
} // namespace under

extern const float LAKES[BIOME_COUNT];
bool lake(int x, int y, float margin);
const Biome *blendPick(int x, int y, uint32_t roll);
const Biome *blendedBiome(int x, int y);

} // namespace world
