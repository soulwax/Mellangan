#include "item/talent/Beacon.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"

Beacon::Beacon()
    : Upgrade("Beacon", atlas::kit(atlas::kt::Sunflower),
              {"Sanctuary heals 10 more and burns brighter", "Sanctuary heals 10 more and burns brighter", "Sanctuary heals 10 more and burns brighter", "Sanctuary heals 10 more and burns brighter", "Sanctuary heals 10 more and burns brighter"},
              false) {
  owner = "PRIESTESS";
}

void Beacon::cast(Player &p) {
  p.heal(10.0f * level);
  p.level->enemiesNear(p.x, p.y, 130, [&](Enemy &e) {
    double xd = e.x - p.x, yd = e.y - p.y;
    if (xd * xd + yd * yd < 110 * 110)
      e.hurt(15.0f * level * p.might, xd, yd, 120, this);
  });
}
