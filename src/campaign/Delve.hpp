#pragma once

#include <vector>

#include "entity/enemy/Enemy.hpp"
#include "level/Edits.hpp"
#include "level/Structure.hpp"

class Level;
struct Campaign;

struct Delve {
  static constexpr int FLOORS = 3, RING = 7;
  static constexpr float REACH = 48, PAUSE = 2.5f;

  bool active = false;
  int floor = 0, rx = 0, ry = 0;
  double x = 0, y = 0;
  float wait = 0, outside = 1, gloom = 0, dust = 0;

  bool start(Level &level, Campaign &c, const Structure &mine);
  void tick(Level &level, Campaign &c);
  void abort(Level &level, Campaign &c);
  int inside(Level &level) const;
  int inside(const Level &level) const { return inside(const_cast<Level &>(level)); }

  static bool entrance(const Level &level, const Campaign &c, Structure &out);
  static EnemyKind guardian(int act);
  static void unseal(Edits &e);

private:
  void seal(Campaign &c);
  void populate(Level &level, Campaign &c);
  void finish(Level &level, Campaign &c);
  void sift(Level &level);
  void quake(Level &level, int strength);
};
