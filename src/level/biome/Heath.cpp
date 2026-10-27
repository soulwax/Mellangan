#include "level/biome/Heath.hpp"

namespace f = feature;

Heath::Heath()
    : Biome({"HEATH", {0.9f, 0.82f, 0.45f, 1}, {1.06f, 1.0f, 0.82f, 1}, 0.24f, 0, 35, 45, {10, 0, 15, 30, 45, 15},
     foe(EnemyKind::Rat) | foe(EnemyKind::Scorpion),
     {{&f::dryBush, 10, 90}, {&f::bush, 6, 50}, {&f::deadPine, 2, 25}, {&f::pine, 2, 25},
      {&f::sprouts, 6, 0}, {&f::log, 4, 0}, {&f::sack, 2, 0}, {&f::crate, 1, 0}, {&f::barrel, 1, 0},
      {&f::crystal, 1, 0}, {&f::heather, 8, 30}, {&f::thornbush, 4, 20}, {&f::boulder, 2, 0}, {&f::pebbles, 2, 0}, {&f::sage, 4, 0}, {&f::rockPile, 2, 0}, {&f::menhir, 1, 0}, {&f::stoneRing, 1, 0}}, &f::heather}) {}
