#include "level/FloorPainter.hpp"

#include "level/Sky.hpp"
#include "level/World.hpp"

namespace {

const int MATERIALS[BIOME_COUNT] = {0, 5, 0, 1, 1, 3, 0, 5, 2, 5, 2, 4, 3,
                                    5, 2, 3, 2, 0, 1, 1, 4, 0, 1, 2, 4, 1};

const int CORNERS[4][5] = {{-1, 1, 24, 0, 8},
                           {1, 1, 26, 12, 8},
                           {-1, -1, 0, 0, 0},
                           {1, -1, 2, 12, 0}};

template <class F> int sides(int x, int y, F at) {
  return int(at(x, y - 1)) | at(x, y + 1) << 1 | at(x - 1, y) << 2 |
         at(x + 1, y) << 3;
}

bool frostless(Sprite ground) {
  return BIOMES[BIOME_SNOW]->frost(ground).y == ground.y;
}

int transitionKind(int id, int variant) {
  const Biome *b = BIOMES[id];
  if (id == BIOME_DESERT)
    return variant < 10 ? 1 : variant < 15 ? 2 : 0;
  if (id == BIOME_ASHLANDS || id == BIOME_CRYSTAL || id == BIOME_FUNGAL ||
      id == BIOME_BADLANDS || id == BIOME_SAVANNA || id == BIOME_TAIGA ||
      id >= BIOME_SALT)
    return variant < 12 ? 1 : variant < 16 ? 2 : 0;
  return variant < b->flowers ? 2 : variant < b->flowers + b->tufts ? 1 : 0;
}

int blockCentre(int v) { return (v - (v < 0 ? 2 : 0)) / 3 * 3 + 1; }

} // namespace

FloorPainter::FloorPainter(Screen &s, const Sky &sky)
    : s(s), sky(sky), snowing(sky.laid == Front::Snow && sky.cover > 0),
      flooded(sky.flood() > 0) {}

void FloorPainter::tint(Color t, float shade, float n, float alpha) {
  s.color({shade * t.r, shade * 0.97f * t.g, (shade * 0.93f + 0.04f * (1 - n)) * t.b,
           alpha});
}

void FloorPainter::tile(int x, int y) {
  float n = world::groundShade(x, y),
        shade = (0.8f + 0.2f * n) * (0.94f + 0.08f * world::heightAt(x, y));
  int own = int(world::biomeAt(x, y));
  Structure st;
  bool wet = world::isWater(x, y);
  if (wet || world::structureAt(x, y, st) || world::isDirt(x, y) ||
      BIOMES[own]->fixedGround(x, y)) {
    if (wet)
      shade *= world::isIce(x, y) ? 0.95f : 0.82f;
    tint(world::groundTint(x, y), shade, n);
    s.sprite(world::groundTile(x, y), x * 16, y * 16, 0, 0);
  } else {
    blend(x, y, own, shade, n);
    if (flooded)
      pool(x, y, shade);
  }
  if (snowing)
    settle(x, y, shade);
  wall(x, y, shade);
}

void FloorPainter::blend(int x, int y, int own, float shade, float n) {
  world::GroundBlend blend = world::groundBlend(x, y);
  int order[3], count = 0;
  for (int i = 0; i < blend.count; i++)
    if (int(blend.biome[i]) == own)
      order[count++] = i;
  for (int i = 0; i < blend.count; i++)
    if (int(blend.biome[i]) != own)
      order[count++] = i;
  float cumulative = 0;
  int variant = int(hash2(x, y) % 100);
  for (int i = 0; i < count; i++) {
    int k = order[i], id = int(blend.biome[k]);
    cumulative += blend.weight[k];
    tint(BIOMES[id]->tint, shade, n, i == 0 ? 1 : blend.weight[k] / cumulative);
    Sprite ground = blend.count == 1 && id == own
                        ? world::groundTile(x, y)
                        : BIOMES[id]->transitionGround(transitionKind(id, variant));
    s.sprite(ground, x * 16, y * 16, 0, 0);
  }
}

void FloorPainter::pool(int x, int y, float shade) {
  if (!sky.poolAt(x, y) || frostless(world::groundTile(x, y)))
    return;
  int mask = sides(x, y, [&](int px, int py) { return sky.poolAt(px, py); });
  s.color({0.4f * shade, 0.47f * shade, 0.64f * shade, 0.94f});
  s.sprite(atlas::waterRim(world::rim(mask)), x * 16, y * 16, 0, 0);
}

void FloorPainter::settle(int x, int y, float shade) {
  auto covered = [&](int px, int py) { return sky.snowAt(px, py); };
  Sprite ground = world::groundTile(x, y),
         white = BIOMES[BIOME_SNOW]->frost(ground);
  if (white.y == ground.y || !covered(x, y))
    return;
  int mask = sides(x, y, covered);
  s.color({shade, shade, shade * 1.03f, 1});
  s.sprite(mask == 15 ? white : atlas::snowRim(world::rim(mask)), x * 16, y * 16,
           0, 0);
}

void FloorPainter::wall(int x, int y, float shade) {
  int level = world::heightAt(x, y);
  if (world::heightAt(x, y - 1) > level && !world::isRamp(x, y - 1))
    s.rect(x * 16, y * 16, 16, 4, {0.02f, 0.02f, 0.06f, 0.3f}),
        s.rect(x * 16, y * 16 + 4, 16, 6, {0.02f, 0.02f, 0.06f, 0.12f});
  if (world::heightAt(x - 1, y) > level)
    s.rect(x * 16, y * 16, 3, 16, {0.02f, 0.02f, 0.06f, 0.16f});
  if (level > 0)
    cliff(x, y, level, shade);
}

void FloorPainter::cliff(int x, int y, int level, float shade) {
  auto top = [&](int nx, int ny) { return world::heightAt(nx, ny) >= level; };
  bool ramp = world::isRamp(x, y);
  int mask = int(top(x, y - 1)) | (top(x, y + 1) || ramp) << 1 |
             top(x - 1, y) << 2 | top(x + 1, y) << 3;
  int biome = world::biomeAt(blockCentre(x), blockCentre(y)), material = MATERIALS[biome];
  Color t = material ? Color{1, 1, 1, 1} : BIOMES[biome]->tint;
  s.color({shade * t.r * 0.92f, shade * t.g * 0.9f, shade * t.b * 0.9f, 1});
  if (ramp) {
    int part = !world::isRamp(x - 1, y) ? 0 : !world::isRamp(x + 1, y) ? 2 : 1;
    s.color({shade, shade, shade, 1});
    s.sprite(atlas::stairs(material, part), x * 16, y * 16, 0, 0);
    return;
  }
  if (mask != 15) {
    s.sprite(atlas::cliff(material, world::rim(mask)), x * 16, y * 16, 0, 0);
    return;
  }
  for (const auto &c : CORNERS) {
    if (top(x + c[0], y + c[1]))
      continue;
    Sprite piece = atlas::cliff(material, c[2]);
    piece.x += float(c[3]), piece.y += float(c[4]), piece.w = 4,
        piece.h = c[1] > 0 ? 8.0f : 4.0f;
    s.sprite(piece, x * 16 + c[3], y * 16 + c[4], 0, 0);
  }
}
