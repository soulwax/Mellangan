#include "level/biome/Meadow.hpp"

namespace f = feature;

Meadow::Meadow()
    : Biome({"MEADOW", {0.55f, 0.9f, 0.35f, 1}, {1, 1, 1, 1}, 0.34f, 6, 22, 55, {50, 10, 15, 15, 10, 5},
     foe(EnemyKind::Rat) | foe(EnemyKind::Slime) | foe(EnemyKind::Knight),
     {{&f::oak, 4, 110}, {&f::bush, 4, 60}, {&f::pine, 2, 50}, {&f::sprouts, 4, 0},
      {&f::beehive, 2, 0}, {&f::sign, 1, 0}, {&f::well, 1, 0}, {&f::log, 1, 0},
      {&f::pinkOak, 1, 20}, {&f::berryBush, 2, 10}}}) {}
