#include "level/biome/Autumn.hpp"

namespace f = feature;

Autumn::Autumn()
    : Biome({"AUTUMN", {1.0f, 0.55f, 0.2f, 1}, {1.04f, 0.94f, 0.8f, 1}, 0.4f, 1, 28, 50, {20, 0, 10, 35, 35, 5},
     foe(EnemyKind::Slime) | foe(EnemyKind::Demon),
     {{&f::autumnOak, 25, 300}, {&f::autumnPine, 20, 260}, {&f::bush, 6, 40}, {&f::dryBush, 4, 30},
      {&f::deadPine, 2, 15}, {&f::mushrooms, 8, 0}, {&f::log, 5, 0}, {&f::sack, 1, 0}}}) {}
