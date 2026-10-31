#include "item/talent/Overcharge.hpp"

#include "entity/Player.hpp"
#include "entity/effect/Strike.hpp"
#include "level/Level.hpp"

Overcharge::Overcharge()
    : Upgrade("Overcharge", atlas::kit(atlas::kt::Amethyst),
              {"Thunderclap calls two more bolts", "Thunderclap calls two more bolts", "Thunderclap calls two more bolts", "Thunderclap calls two more bolts", "Thunderclap calls two more bolts"},
              false) {
  owner = "STORMCALLER";
}

void Overcharge::cast(Mob &p) {
  auto targets = p.level->nearestFoes(p, p.x, p.y, 260, 2 * level);
  for (Mob *e : targets)
    p.level->add<Strike>(e->x, e->y, 45 * p.might, 24)->source = this;
}
