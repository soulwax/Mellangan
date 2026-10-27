#pragma once

#include <cmath>

#include "level/Biome.hpp"
#include "level/Prop.hpp"
#include "level/Structure.hpp"

namespace world {

constexpr int REGION = 20;

inline int regionOf(int tile) { return int(std::floor(float(tile) / REGION)); }
bool structureIn(int rx, int ry, Structure &out);
bool structureAt(int x, int y, Structure &out, int margin = 0);
bool nearestStructure(Vec2 p, int maxRegions, Structure &out);

bool isDirt(int x, int y);
Sprite groundTile(int x, int y);
float groundShade(int x, int y);
Color groundTint(int x, int y);
bool propAt(int x, int y, Prop &p);

} // namespace world
