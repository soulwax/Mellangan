#pragma once

#include <cmath>
#include <cstdint>

#include "level/Prop.hpp"
#include "level/Structure.hpp"
#include "level/biome/Biome.hpp"


namespace world {

constexpr int REGION = 20;

enum class Layer : uint8_t { Surface, Caves, Deep };
Layer layer();
void enter(Layer to);

enum class CaveKind : uint8_t { Solid, Pocket, Through, Warren, Descent };
constexpr int CAVE_CELL = 64;
struct CaveCell {
  int cx, cy;
  CaveKind kind;
  int size, hx, hy, mouths;
  int mx[3], my[3];
  int px, py;
};
bool caveCell(int cx, int cy, CaveCell &out);
bool isRock(int x, int y);
bool caveMouth(int x, int y);
bool caveMouthNear(double x, double y, int &tx, int &ty);
void openCaves(bool open);

// Floor division: bias negatives by REGION - 1 (sign mask & 19) so truncation
// rounds down.
inline int regionOf(int tile) {
  return (tile - (tile >> 31 & (REGION - 1))) / REGION;
}
inline int tileOf(double v) { return int(std::floor(v / 16)); }
inline BiomeId biomeUnder(double x, double y) { return biomeAt(tileOf(x), tileOf(y)); }

bool structureIn(int rx, int ry, Structure &out);
bool structureAt(int x, int y, Structure &out, int margin = 0);
typedef bool (*StructureFilter)(const Structure *s, const void *ctx);
bool nearestStructure(double x, double y, int maxRegions, Structure &out,
                      StructureFilter keep = nullptr,
                      const void *ctx = nullptr);

typedef bool (*PatchTest)(int x, int y, const void *ctx);
bool smoothPatch(PatchTest test, const void *ctx, int x, int y);

struct GroundBlend {
  BiomeId biome[3];
  float weight[3];
  int count;
};

GroundBlend groundBlend(int x, int y);
bool isDirt(int x, int y);
bool isWater(int x, int y);
bool isIce(int x, int y);
int heightAt(int x, int y);
bool isRamp(int x, int y);
Sprite groundTile(int x, int y);
float groundShade(int x, int y);
Color groundTint(int x, int y);
bool propAt(int x, int y, Prop &p);
bool findBiome(int biome, int &x, int &y, int reach = 4000);

} // namespace world
