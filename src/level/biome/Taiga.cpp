#include "level/biome/Taiga.hpp"

namespace f = feature;

Taiga::Taiga()
    : Carpet(
          {"TAIGA",
           {0.25f, 0.5f, 0.42f, 1},
           {0.9f, 0.96f, 0.94f, 1},
           2.0f,
           0,
           0,
           45,
           {20, 10, 15, 30, 10, 5, 20, 10},
           foe(EnemyKind::Bat) | foe(EnemyKind::Spider) | foe(EnemyKind::Slime),
           {{&f::taigaSpruce, 28, 320},
            {&f::larch, 5, 60},
            {&f::fir, 4, 40},
            {&f::lingonberry, 4, 10},
            {&f::lichenRock, 3, 0},
            {&f::mossyLog, 3, 0},
            {&f::shelfStump, 3, 0},
            {&f::fern, 4, 20},
            {&f::cottonGrass, 3, 0},
            {&f::needleTuft, 6, 0},
            {&f::brownMushrooms, 2, 0},
            {&f::blueberries, 2, 0}},
           &f::needleTuft},
          BIOME_TAIGA, atlas::cp::Needles) {}
