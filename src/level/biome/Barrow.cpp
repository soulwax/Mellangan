#include "level/biome/Barrow.hpp"

namespace f = feature;

Barrow::Barrow()
    : Biome({"BARROW",
             {0.6f, 0.55f, 1.0f, 1},
             {0.8f, 0.86f, 1.0f, 1},
             0.3f,
             0,
             12,
             55,
             {0, 65, 25, 0, 10},
             foe(EnemyKind::Bat) | foe(EnemyKind::Ghost) |
                 foe(EnemyKind::Cultist),
             {{&f::grave, 8, 90},
              {&f::deadPine, 6, 60},
              {&f::bush, 4, 30},
              {&f::spiritPine, 2, 25},
              {&f::autumnPine, 2, 20},
              {&f::mushrooms, 4, 0},
              {&f::blueShrooms, 3, 0},
              {&f::crystal, 3, 0},
              {&f::charredTree, 3, 30},
              {&f::ashTuft, 4, 0},
              {&f::thornbush, 2, 0},
              {&f::obsidian, 1, 0},
              {&f::deadOak, 4, 40},
              {&f::skull, 2, 0},
              {&f::menhir, 2, 0},
              {&f::glowshrooms, 2, 0},
              {&f::stoneRing, 1, 0}},
             &f::ashTuft}) {}
