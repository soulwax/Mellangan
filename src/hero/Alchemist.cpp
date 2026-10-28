#include "hero/Alchemist.hpp"

#include "entity/Player.hpp"
#include "entity/pickup/Potion.hpp"
#include "level/Level.hpp"

namespace {
bool hoarder(const Codex &c) { return c.count(c.relics) >= 3; }
} // namespace

Alchemist::Alchemist()
    : Hero({"ALCHEMIST", "Fire Bomb", "Brewer of fire and cures",
            "Fallen foes may leave a potion", "find 3 relics", hoarder, 100,
            0, 1, 0, 0, 14, 0, 0.5f, 0}) {}

void Alchemist::slay(Player &p, Enemy &e) const {
  if (rng.chance(0.03f))
    p.level->add<Potion>(e.x, e.y);
}
