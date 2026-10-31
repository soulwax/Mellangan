#pragma once

#include "entity/enemy/Enemy.hpp"

struct Phase {
  const Cast *casts;
  int count;
};

struct BossDef {
  EnemyKind kind;
  const char *title, *arrival, *fall;
  Color tone;
  Phase phases[3];
  EnemyKind minion;
  int minions;
  uint32_t lands;
};

constexpr int BOSS_COUNT = 9;
extern const BossDef BOSS_DEFS[BOSS_COUNT];

const BossDef &bossOf(EnemyKind kind);
const BossDef *bossFor(int biome);
