#include "level/biome/Desert.hpp"

#include "level/Noise.h"

namespace f = feature;
namespace fl = atlas::fl;

namespace {

constexpr float WATER = 0.31f, GREEN = 0.25f;

float oasis(int x, int y) {
    struct Slot {
        int x, y;
        float v;
        bool used;
    };
    static Slot cache[128 * 64];
    Slot *c = &cache[(x & 127) | (y & 63) << 7];
    if (!c->used || c->x != x || c->y != y) *c = {x, y, noise_fbm(x * 0.06f + 300.5f, y * 0.06f - 120.5f, 2, 909), true};
    return c->v;
}

bool desert(int x, int y) { return world::biomeAt(x, y) == BIOME_DESERT; }
bool pool(int x, int y) { return desert(x, y) && oasis(x, y) > WATER; }
bool dune(int x, int y) { return desert(x, y) && oasis(x, y) <= GREEN; }

bool block(bool (*f)(int, int), int x, int y) {
    if (!f(x, y)) return false;
    for (int dy = -1; dy <= 0; dy++)
        for (int dx = -1; dx <= 0; dx++)
            if (f(x + dx, y + dy) && f(x + dx + 1, y + dy) && f(x + dx, y + dy + 1) && f(x + dx + 1, y + dy + 1)) return true;
    return false;
}

bool wet(int x, int y) { return block(pool, x, y); }
bool sand(int x, int y) { return block(dune, x, y); }

} // namespace

Desert::Desert()
    : Biome({"DESERT", {0.95f, 0.85f, 0.5f, 1}, {1.06f, 1.0f, 0.9f, 1}, 2.0f, 0, 0, 45, {0, 5, 20, 35, 40, 0},
             foe(EnemyKind::Scorpion) | foe(EnemyKind::Rat) | foe(EnemyKind::Demon),
             {{&f::cactus, 10, 50}, {&f::barrelCactus, 6, 25}, {&f::dryBush, 6, 30}, {&f::deadPine, 1, 6},
              {&f::sack, 1, 0}, {&f::crystal, 1, 0}, {&f::sandstone, 3, 0}, {&f::pebbles, 2, 0}, {&f::dryGrass, 2, 0}, {&f::acacia, 1, 8}, {&f::baobab, 0, 4}}, &f::sandTuft}) {}

bool Desert::ground(int x, int y, Sprite *out) const {
    if (wet(x, y)) {
        int mask = int(wet(x, y - 1)) | wet(x, y + 1) << 1 | wet(x - 1, y) << 2 | wet(x + 1, y) << 3;
        *out = atlas::waterRim(world::rim(mask));
        return true;
    }
    if (!sand(x, y)) return false;
    int mask = int(sand(x, y - 1)) | sand(x, y + 1) << 1 | sand(x - 1, y) << 2 | sand(x + 1, y) << 3;
    if (mask != 15) {
        *out = atlas::sandRim(world::rim(mask));
        return true;
    }
    int r = hash2(x, y) % 100;
    *out = atlas::flora(r < 10 ? fl::SandSpeck : r < 15 ? fl::SandPebbles : fl::Sand);
    return true;
}

int Desert::place(int x, int y, uint32_t roll, Prop &p) const {
    if (wet(x, y)) return p.solid = 7, 1;
    if (sand(x, y)) return -1;
    int r = int(roll % 1000);
    const Feature *pick = r < 200 ? &f::palm : r < 230 ? &f::jungleTree : r < 290 ? &f::reeds : r < 330 ? &f::cattails
                        : r < 360 ? &f::bush : r < 380 ? &f::berryBush : r < 400 ? &f::fern : nullptr;
    return pick ? int(pick->place(pick, roll >> 10, &p)) : 0;
}
