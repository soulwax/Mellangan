#pragma once

#include <mutex>

#include "entity/enemy/Enemy.hpp"

class Level;

class Spawner {
public:
  explicit Spawner(Level &level) : level(level) {}
  void tick();
  Enemy *spawn(EnemyKind kind, double x, double y, bool aware = true,
               bool boss = false);
  void wave(int index, int count);
  Enemy *champion(double x, double y);
  static constexpr int WAVES = 6;

private:
  Level &level;
  float timer = 1, nextWave = 60, nextBoss = 150, nextBarrel = 15,
        nextMimic = 70, nextBand = 160, nextChampion = 110;
  int bosses = 0, lastWave = -1;

  void ring(float dist, double &x, double &y) const;
  std::mutex spawnMutex;
};
