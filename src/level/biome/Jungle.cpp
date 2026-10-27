#include "level/biome/Jungle.hpp"

namespace f = feature;

Jungle::Jungle()
    : Biome(
          {"JUNGLE",
           {0.15f, 0.6f, 0.2f, 1},
           {0.82f, 0.96f, 0.8f, 1},
           0.3f,
           2,
           30,
           50,
           {0, 0, 20, 30, 50, 0},
           foe(EnemyKind::Spider) | foe(EnemyKind::Slime) | foe(EnemyKind::Bat),
           {{&f::jungleTree, 25, 300},
            {&f::treeFern, 10, 120},
            {&f::bamboo, 8, 100},
            {&f::fern, 10, 40},
            {&f::ivy, 4, 30},
            {&f::mossyLog, 3, 0},
            {&f::glowshrooms, 2, 0},
            {&f::blueShrooms, 2, 0},
            {&f::callaLilies, 2, 0},
            {&f::palm, 1, 10}},
           &f::tallGrass}) {}

Sprite Jungle::dirtRim(int offset) const { return atlas::mudRim(offset); }
