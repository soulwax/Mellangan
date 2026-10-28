#pragma once

#include <mutex>

#include "entity/enemy/Enemy.hpp"

class Level;

class Spawner {
public:
  explicit Spawner(Level &level) : level(level) {}
  void tick();
  void spawn(EnemyKind kind, double x, double y, bool aware = true);

private:
  Level &level;
  float timer = 1, nextWave = 60, nextBoss = 150, nextBarrel = 15;
  int bosses = 0;

  void ring(float dist, double &x, double &y) const;
  std::mutex spawnMutex;
};
