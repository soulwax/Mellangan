#include "item/talent/Harvest.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"

Harvest::Harvest()
    : Upgrade("Harvest", atlas::kit(atlas::kt::Wheat),
              {"Every kill shortens Death's Embrace", "Every kill shortens Death's Embrace", "Every kill shortens Death's Embrace", "Every kill shortens Death's Embrace", "Every kill shortens Death's Embrace"},
              false) {
  owner = "REAPER";
}

void Harvest::slay(Player &p, Enemy &) {
  p.skillTime = std::max(0.0f, p.skillTime - 0.15f * level);
}
