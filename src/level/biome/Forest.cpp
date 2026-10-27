#include "level/biome/Forest.hpp"

namespace f = feature;

Forest::Forest()
    : Biome({"FOREST", {0.2f, 0.7f, 0.3f, 1}, {0.88f, 0.96f, 0.86f, 1}, 0.42f, 1, 30, 50, {5, 0, 10, 45, 40},
     foe(EnemyKind::Bat) | foe(EnemyKind::Spider),
     {{&f::oak, 30, 330}, {&f::pine, 25, 300}, {&f::mossPine, 10, 120}, {&f::bush, 10, 80},
      {&f::mushrooms, 6, 0}, {&f::blueShrooms, 2, 0}, {&f::berryBush, 2, 0}, {&f::log, 4, 0}, {&f::sprouts, 3, 0}, {&f::birch, 6, 60}, {&f::fir, 4, 40}, {&f::fern, 6, 20}, {&f::mossyBoulder, 2, 0}}, &f::fern}) {}
