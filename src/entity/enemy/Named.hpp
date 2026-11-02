#pragma once

#include "entity/Trait.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/biome/Biome.hpp"

class Level;

struct NamedPack {
  const char *name;
  EnemyKind base;
  Trait traits[2];
  EnemyKind minion;
  int count;
  BiomeId home;
};

constexpr int NAMED_COUNT = 10;
extern const NamedPack NAMED[NAMED_COUNT];

namespace named {

void tick(Level &level);
Enemy *raise(Level &level, int index, double x, double y);
void fall(Enemy &leader);

} // namespace named
