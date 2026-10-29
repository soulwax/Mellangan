#include "level/biome/Desert.hpp"

#include "level/Noise.h"
#include "level/World.hpp"

namespace f = feature;
namespace fl = atlas::fl;

namespace {

constexpr float WATER = 0.31f, GREEN = 0.25f;

float oasis(int x, int y) {
  struct Slot {
    int x, y;
    float v;
    uint32_t epoch;
  };
  static Slot cache[128 * 64];
  Slot *c = &cache[(x & 127) | (y & 63) << 7];
  if (c->epoch != noise_epoch || c->x != x || c->y != y)
    *c = {x, y, noise_fbm(x * 0.06f + 300.5f, y * 0.06f - 120.5f, 2, 909),
          noise_epoch};
  return c->v;
}

bool desert(int x, int y) { return world::biomeAt(x, y) == BIOME_DESERT; }
bool pool(int x, int y) { return desert(x, y) && oasis(x, y) > WATER; }
bool dune(int x, int y) { return desert(x, y) && oasis(x, y) <= GREEN; }

bool call(int x, int y, const void *f) {
  return reinterpret_cast<bool (*)(int, int)>(const_cast<void *>(f))(x, y);
}

bool block(bool (*f)(int, int), int x, int y) {
  return world::smoothPatch(call, reinterpret_cast<const void *>(f), x, y);
}

bool wet(int x, int y) { return block(pool, x, y); }
bool sand(int x, int y) { return block(dune, x, y); }

} // namespace

Desert::Desert()
    : Biome({"DESERT",
             {0.95f, 0.85f, 0.5f, 1},
             {1.06f, 1.0f, 0.9f, 1},
             2.0f,
             0,
             0,
             45,
             {0, 5, 20, 35, 40, 0, 0, 20},
             foe(EnemyKind::Scorpion) | foe(EnemyKind::Rat) |
                 foe(EnemyKind::Burrower) | foe(EnemyKind::Demon),
             {{&f::cactus, 10, 50},
              {&f::barrelCactus, 6, 25},
              {&f::dryBush, 6, 30},
              {&f::deadPine, 1, 6},
              {&f::sack, 1, 0},
              {&f::crystal, 1, 0},
              {&f::sandstone, 3, 0},
              {&f::pebbles, 2, 0},
              {&f::dryGrass, 2, 0},
              {&f::acacia, 1, 8},
              {&f::baobab, 0, 4},
              {&f::joshuaTree, 3, 25},
              {&f::pricklyPear, 4, 0},
              {&f::agave, 3, 0},
              {&f::tumbleweed, 2, 0},
              {&f::skull, 1, 0},
              {&f::amethyst, 0, 2}},
             &f::sandTuft}) {}

bool Desert::ground(int x, int y, Sprite *out) const {
  if (wet(x, y)) {
    int mask = int(wet(x, y - 1)) | wet(x, y + 1) << 1 | wet(x - 1, y) << 2 |
               wet(x + 1, y) << 3;
    *out = atlas::waterRim(world::rim(mask));
    return true;
  }
  if (!sand(x, y))
    return false;
  int mask = int(sand(x, y - 1)) | sand(x, y + 1) << 1 | sand(x - 1, y) << 2 |
             sand(x + 1, y) << 3;
  if (mask != 15) {
    *out = atlas::sandRim(world::rim(mask));
    return true;
  }
  int r = hash2(x, y) % 100;
  *out = atlas::flora(r < 10   ? fl::SandSpeck
                      : r < 15 ? fl::SandPebbles
                               : fl::Sand);
  return true;
}

bool Desert::fixedGround(int x, int y) const { return wet(x, y); }

int Desert::place(int x, int y, uint32_t roll, Prop &p) const {
  if (wet(x, y))
    return p.solid = 7, 1;
  if (sand(x, y))
    return -1;
  int r = int(roll % 1000);
  const Feature *pick = r < 180   ? &f::palm
                        : r < 205 ? &f::jungleTree
                        : r < 225 ? &f::bamboo
                        : r < 245 ? &f::treeFern
                        : r < 295 ? &f::reeds
                        : r < 330 ? &f::cattails
                        : r < 355 ? &f::bush
                        : r < 375 ? &f::berryBush
                        : r < 395 ? &f::fern
                        : r < 410 ? &f::callaLilies
                                  : nullptr;
  return pick ? int(pick->place(pick, roll >> 10, &p)) : 0;
}
