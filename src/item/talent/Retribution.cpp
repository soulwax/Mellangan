#include "item/talent/Retribution.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"

Retribution::Retribution()
    : Upgrade("Retribution", atlas::kit(atlas::kt::Holly),
              {"Blows against you strike back at all near", "Blows against you strike back at all near", "Blows against you strike back at all near", "Blows against you strike back at all near", "Blows against you strike back at all near"},
              false) {
  owner = "KNIGHT";
}

float Retribution::guard(Player &p, float damage) {
  p.level->enemiesNear(p.x, p.y, 80, [&](Enemy &e) {
    double xd = e.x - p.x, yd = e.y - p.y;
    if (xd * xd + yd * yd < 60 * 60)
      e.hurt(25.0f * level * p.might, xd, yd, 160, this);
  });
  return damage;
}
