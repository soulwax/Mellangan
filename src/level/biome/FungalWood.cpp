#include "level/biome/FungalWood.hpp"

#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace f = feature;

FungalWood::FungalWood()
    : Carpet({"FUNGAL",
              {0.75f, 0.45f, 0.7f, 1},
              {0.96f, 0.9f, 1.0f, 1},
              2.0f,
              0,
              0,
              50,
              {5, 10, 25, 20, 40, 0},
              foe(EnemyKind::Slime) | foe(EnemyKind::Spider) |
                  foe(EnemyKind::Ghost),
              {{&f::mushroomTree, 10, 140},
               {&f::brownMushrooms, 8, 30},
               {&f::glowshrooms, 6, 20},
               {&f::toadstools, 5, 0},
               {&f::blueShrooms, 4, 0},
               {&f::mushrooms, 4, 0},
               {&f::stump, 2, 0},
               {&f::mossyLog, 2, 0}},
              &f::brownMushrooms},
             BIOME_FUNGAL, atlas::cp::Mycelium) {}

void FungalWood::weather(Level &level, double x, double y) const {
  if (Spark::alive > 500 || rng.unit() > 0.3f)
    return;
  level.add<Spark>(x + rng.range(-260, 260), y + rng.range(-160, 180),
                   rng.range(-5, 5), rng.range(-8, -2),
                   Color{0.95f, 0.75f, 0.9f, 1}, rng.range(2.0f, 3.0f), 0.25f,
                   rng.chance(0.4f));
}
