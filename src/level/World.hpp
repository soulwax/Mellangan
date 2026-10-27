#pragma once

#include <cmath>

#include "level/Biome.hpp"
#include "level/Prop.hpp"
#include "level/Structure.hpp"

namespace world {

constexpr int REGION = 20;

// Floor division: bias negatives by REGION - 1 (sign mask & 19) so truncation rounds down.
inline int regionOf(int tile) { return (tile - (tile >> 31 & (REGION - 1))) / REGION; }
bool structureIn(int rx, int ry, Structure &out);
bool structureAt(int x, int y, Structure &out, int margin = 0);
typedef bool (*StructureFilter)(const Structure *s, const void *ctx);
bool nearestStructure(double x, double y, int maxRegions, Structure &out, StructureFilter keep = nullptr, const void *ctx = nullptr);

bool isDirt(int x, int y);
Sprite groundTile(int x, int y);
float groundShade(int x, int y);
Color groundTint(int x, int y);
bool propAt(int x, int y, Prop &p);

} // namespace world
