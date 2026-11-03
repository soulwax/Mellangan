#pragma once

#include <mutex>
#include <vector>

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
  static constexpr float MINI_GAP = 150, WARD = 0.7f;

private:
  Level &level;
  float timer = 1, nextWave = 60, nextBoss = 150, nextBarrel = 15,
        nextMimic = 70, nextBand = 160, nextChampion = 110, nextMini = 0;
  int bosses = 0, lastWave = -1;

  struct Roster {
    std::vector<EnemyKind> pool, common;
  };

  Roster roster(float t) const;
  EnemyKind pick(const Roster &r, double x, double y) const;
  void trickle(float t);
  void horde(float t);
  void mimics();
  void renegades(float t);
  void champions();
  void barrel();
  void boss();
  void ring(float dist, double &x, double &y) const;
  std::mutex spawnMutex;
};
