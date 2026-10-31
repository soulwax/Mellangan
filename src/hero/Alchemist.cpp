#include "hero/Alchemist.hpp"

#include "entity/Ability.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "entity/pickup/Potion.hpp"

namespace {
bool hoarder(const Codex &c) { return c.count(c.relics) >= 3; }
} // namespace

Alchemist::Alchemist()
    : Hero({"ALCHEMIST", "Fire Bomb", "Brewer of fire and cures",
            "Fallen foes may leave a potion", "find 3 relics", hoarder, 100,
            {1, 0.6f, 0.3f, 1}, 0, 1, 0, 0, 14, 0, 0.5f, 0, "ELIXIR",
            "Drink to heal 25, hurl three bombs", 12,
            {"Fire Bomb", "Troll Blood", "Lodestone"}}) {}

void Alchemist::cast(Mob &p) const {
  ability::bombs(p, 3, 20, 25);
}

void Alchemist::slay(Mob &p, Enemy &e) const {
  if (rng.chance(0.03f))
    p.level->add<Potion>(e.x, e.y);
}
