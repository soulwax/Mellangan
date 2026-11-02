#include "entity/effect/Boon.hpp"

#include "level/Level.hpp"
#include "particle/Spark.hpp"

Boon::Boon(const Mob &target, float Mob::*stat, float delta, float seconds, float delay,
           Color tone)
    : target(target.id), stat(stat), delta(delta), life(seconds), delay(delay), tone(tone) {
  x = target.x, y = target.y;
}

void Boon::tick() {
  Mob *m = level->mob(target);
  if (!m || m->dead())
    return remove();
  x = m->x, y = m->y;
  if (delay > 0) {
    delay -= DT;
    return;
  }
  if (!on)
    on = true, m->*stat += delta;
  if ((life -= DT) <= 0) {
    m->*stat -= delta;
    return remove();
  }
  if (rng.chance(0.12f) && Spark::alive < 600)
    level->add<Spark>(m->x + rng.range(-6, 6), m->bodyY() + rng.range(-4, 4), 0,
                      -rng.range(20, 40), tone, 0.35f, 0.4f, true);
}
