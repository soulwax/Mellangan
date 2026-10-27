#include "level/biome/Blossom.hpp"

namespace f = feature;

Blossom::Blossom()
    : Biome(
          {"BLOSSOM",
           {1.0f, 0.6f, 0.85f, 1},
           {1.03f, 0.97f, 1.01f, 1},
           0.38f,
           12,
           22,
           55,
           {40, 0, 30, 10, 15, 5},
           foe(EnemyKind::Bat) | foe(EnemyKind::Slime) | foe(EnemyKind::Spider),
           {{&f::pinkOak, 12, 200},
            {&f::whiteOak, 7, 110},
            {&f::lilac, 6, 60},
            {&f::berryBush, 3, 20},
            {&f::oak, 2, 20},
            {&f::beehive, 2, 0},
            {&f::sprouts, 4, 0},
            {&f::well, 1, 0},
            {&f::sign, 1, 0},
            {&f::lavender, 4, 0},
            {&f::tulips, 3, 0},
            {&f::daisies, 3, 0},
            {&f::cypress, 1, 10},
            {&f::appleTree, 2, 20},
            {&f::roses, 3, 0},
            {&f::hydrangea, 2, 0},
            {&f::callaLilies, 2, 0},
            {&f::strawberries, 2, 0}},
           &f::daisies}) {}

Sprite Blossom::grass(int kind) const {
  if (kind == 0)
    return Biome::grass(0);
  return atlas::flora(kind == 2 ? atlas::fl::Petals : atlas::fl::PetalTufts);
}
