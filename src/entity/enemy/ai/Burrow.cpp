#include "entity/enemy/ai/Burrow.hpp"

#include <cmath>

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/SmokeDebris.hpp"

void Burrow::start(Enemy &e, const Sense &s) const {
  e.buried = true;
  e.xMark = e.x + s.xd;
  e.yMark = e.y + s.yd;
}

GoalState Burrow::tick(Enemy &e, const Sense &s, double &xs, double &ys) const {
  e.aware = true;
  if (e.buried) {
    if (e.goalTime < travel) {
      if (s.dist <= 30) {
        e.goalTime = travel;
        Player &p = *e.level->player;
        e.xMark = p.x;
        e.yMark = p.bodyY();
      } else {
        double spd = e.type.speed * 1.35;
        xs = s.xn * spd;
        ys = s.yn * spd;
        e.xMark = e.x + s.xd;
        e.yMark = e.y + s.yd;
        if (e.level->ticks % 5 == 0 && Debris::room())
          e.level->add<SmokeDebris>(e.x, e.y, 1);
        return GoalState::Running;
      }
    }
    xs = ys = 0;
    if (e.goalTime >= travel + warn) {
      e.x = e.xMark;
      e.y = e.yMark;
      e.buried = false;
      Sound::hit.play(0.6f, -0.15f);
      Spark::burst(*e.level, e.x, e.bodyY(), e.type.blood, 25, 110, true);

      Player &p = *e.level->player;
      double dx = p.x - e.x, dy = p.y - e.y;
      if (dx * dx + dy * dy < 28 * 28 && !p.dead()) {
        p.felledBy = e.name();
        p.hurt(e.type.damage * e.damageMul * 1.25f);
      }
    }
    return GoalState::Running;
  }

  if (e.goalTime >= travel + warn + surface)
    return GoalState::Done;
  xs = s.xn * e.type.speed * 0.4;
  ys = s.yn * e.type.speed * 0.4;
  return GoalState::Running;
}

float Burrow::tell(const Enemy &e) const {
  if (e.buried && e.goalTime >= travel)
    return std::clamp((e.goalTime - travel) / warn, 0.0f, 1.0f);
  return 0.0f;
}
