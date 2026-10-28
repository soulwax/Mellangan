#include "level/biome/CrystalFields.hpp"

#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace f = feature;

CrystalFields::CrystalFields()
    : Carpet({"CRYSTAL",
              {0.7f, 0.6f, 1.0f, 1},
              {0.94f, 0.92f, 1.06f, 1},
              2.0f,
              0,
              0,
              50,
              {0, 0, 50, 10, 30, 10, 30},
              foe(EnemyKind::Ghost) | foe(EnemyKind::Cultist) |
                  foe(EnemyKind::Knight),
              {{&f::crystalSpire, 6, 60},
               {&f::amethyst, 6, 20},
               {&f::iceChunk, 3, 0},
               {&f::crystal, 3, 0},
               {&f::frostBirch, 2, 20},
               {&f::snowdrops, 2, 0},
               {&f::menhir, 1, 0}},
              &f::amethyst},
             BIOME_CRYSTAL, atlas::cp::CrystalMoss) {}

void CrystalFields::weather(Level &level, double x, double y) const {
  if (Spark::alive > 500 || rng.unit() > 0.35f)
    return;
  level.add<Spark>(x + rng.range(-260, 260), y + rng.range(-160, 180),
                   rng.range(-2, 2), rng.range(-4, 0),
                   Color{0.85f, 0.8f, 1.0f, 1}, rng.range(0.5f, 1.0f), 0.3f,
                   true);
}
