#include "item/talent/SecondWind.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"

SecondWind::SecondWind()
    : Upgrade("Second Wind", atlas::kit(atlas::kt::Pampas),
              {"Kills may instantly reset Volley", "Kills may instantly reset Volley", "Kills may instantly reset Volley", "Kills may instantly reset Volley", "Kills may instantly reset Volley"},
              false) {
  owner = "RANGER";
}

void SecondWind::slay(Mob &p, Enemy &) {
  if (p.skillTime > 0 && rng.chance(0.03f * level))
    p.skillTime = 0;
}
