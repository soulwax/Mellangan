#include "level/biome/Ashlands.hpp"

#include "level/Level.hpp"
#include "level/Noise.h"
#include "particle/Spark.hpp"

namespace f = feature;

Ashlands::Ashlands()
    : Carpet({"ASHLANDS",
              {0.9f, 0.35f, 0.15f, 1},
              {0.96f, 0.86f, 0.82f, 1},
              2.0f,
              0,
              0,
              45,
              {0, 10, 20, 10, 60, 0, 0, 20},
              foe(EnemyKind::Demon) | foe(EnemyKind::Cultist) |
                  foe(EnemyKind::Bat),
              {{&f::charredTree, 8, 80},
               {&f::obsidian, 4, 20},
               {&f::ashTuft, 8, 0},
               {&f::rockPile, 3, 0},
               {&f::skull, 1, 0},
               {&f::deadOak, 1, 6},
               {&f::deadPine, 1, 6}},
              &f::ashTuft},
             BIOME_ASHLANDS, atlas::cp::Ash) {}

namespace {

bool molten(int x, int y, BiomeId id) {
  auto own = [id](int ox, int oy) { return world::biomeAt(ox, oy) == id; };
  return own(x, y) && own(x - 1, y) && own(x + 1, y) && own(x, y - 1) &&
         own(x, y + 1) &&
         noise_fbm(x * 0.08f + 77.5f, y * 0.08f - 31.5f, 2, 1313) > 0.24f;
}

bool lava(int x, int y) { return blockOf(molten, BIOME_ASHLANDS, x, y); }

} // namespace

bool Ashlands::ground(int x, int y, Sprite *out) const {
  if (!lava(x, y))
    return Carpet::ground(x, y, out);
  *out = edge(atlas::cp::Lava, int(lava(x, y - 1)) | lava(x, y + 1) << 1 |
                                   lava(x - 1, y) << 2 | lava(x + 1, y) << 3);
  return true;
}

bool Ashlands::fixedGround(int x, int y) const { return lava(x, y); }

int Ashlands::place(int x, int y, uint32_t roll, Prop &p) const {
  if (!lava(x, y))
    return -1;
  p.solid = 7;
  if (roll % 3 == 0)
    p.light = {0.5f, 0.18f, 0.04f, 1}, p.lightSize = 70, p.lightYo = -6,
    p.wisp = false;
  return 1;
}

void Ashlands::weather(Level &level, double x, double y) const {
  if (Spark::alive > 500 || rng.unit() > 0.5f)
    return;
  level.add<Spark>(x + rng.range(-260, 260), y + rng.range(-140, 200),
                   rng.range(-8, 8), rng.range(-28, -12),
                   Color{1.0f, 0.5f, 0.15f, 1}, rng.range(1.0f, 2.2f), 0.3f,
                   rng.chance(0.3f));
}
