#include "level/biome/Savanna.hpp"

namespace f = feature;

Savanna::Savanna()
    : Carpet({"SAVANNA",
              {0.85f, 0.75f, 0.3f, 1},
              {1.04f, 1.0f, 0.9f, 1},
              0.32f,
              0,
              0,
              45,
              {10, 0, 15, 35, 40, 0},
              foe(EnemyKind::Rat) | foe(EnemyKind::Scorpion) |
                  foe(EnemyKind::Demon),
              {{&f::acacia, 6, 60},
               {&f::baobab, 2, 15},
               {&f::dryGrass, 10, 40},
               {&f::pampas, 4, 0},
               {&f::wheat, 2, 0},
               {&f::tallGrass, 3, 0},
               {&f::sandstone, 2, 0},
               {&f::rockPile, 1, 0},
               {&f::tumbleweed, 1, 0},
               {&f::dryBush, 3, 20}},
              &f::pampas},
             BIOME_SAVANNA, atlas::cp::Straw) {}
