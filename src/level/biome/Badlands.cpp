#include "level/biome/Badlands.hpp"

namespace f = feature;

Badlands::Badlands()
    : Carpet({"BADLANDS",
              {0.85f, 0.42f, 0.28f, 1},
              {1.06f, 0.96f, 0.9f, 1},
              2.0f,
              0,
              0,
              45,
              {0, 10, 15, 30, 25, 0, 0, 45},
              foe(EnemyKind::Scorpion) | foe(EnemyKind::Rat) |
                  foe(EnemyKind::Burrower) | foe(EnemyKind::Cyclops),
              {{&f::mesa, 5, 50},
               {&f::hoodoo, 5, 30},
               {&f::juniper, 4, 30},
               {&f::ocotillo, 4, 10},
               {&f::redRocks, 6, 0},
               {&f::clayTuft, 6, 0},
               {&f::ribs, 1, 0},
               {&f::skull, 1, 0},
               {&f::sandstone, 2, 0},
               {&f::tumbleweed, 2, 0},
               {&f::dryBush, 2, 10},
               {&f::agave, 1, 0}},
              &f::redRocks},
             BIOME_BADLANDS, atlas::cp::RedClay) {}
