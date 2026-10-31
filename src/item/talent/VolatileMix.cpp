#include "item/talent/VolatileMix.hpp"

#include "entity/Player.hpp"
#include "particle/Explosion.hpp"
#include "level/Level.hpp"

VolatileMix::VolatileMix()
    : Upgrade("Volatile Mix", atlas::kit(atlas::kt::Pumpkins),
              {"Fallen foes may burst into flame", "Fallen foes may burst into flame", "Fallen foes may burst into flame", "Fallen foes may burst into flame", "Fallen foes may burst into flame"},
              false) {
  owner = "ALCHEMIST";
}

void VolatileMix::slay(Mob &p, Enemy &e) {
  if (!rng.chance(0.08f * level))
    return;
  double x = e.x, y = e.y;
  p.level->add<Explosion>(x, y, 4, 5);
  p.level->foes(p, x, y, 60, [&](Mob &o) {
    double xd = o.x - x, yd = o.y - y;
    if (&o != &e && xd * xd + yd * yd < 44 * 44)
      o.strike(25 * p.might, xd, yd, 90, this);
  });
}
