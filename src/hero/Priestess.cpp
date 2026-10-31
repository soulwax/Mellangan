#include "hero/Priestess.hpp"

#include "entity/Ability.hpp"
#include "entity/Player.hpp"

namespace {
bool devout(const Codex &c) { return c.shrines >= 10; }
} // namespace

Priestess::Priestess()
    : Hero({"PRIESTESS", "Warding Halo", "Lantern of the long night",
            "Shrines kindle 30% faster", "kindle 10 shrines", devout, 99,
            {1, 0.92f, 0.6f, 1}, 10, 1, 0, 0, 0, 0, 0.3f, 0, "SANCTUARY",
            "Heal 35 and drive back the dark", 14,
            {"Warding Halo", "Heartroot", "Troll Blood"}}) {}

void Priestess::cast(Player &p) const {
  ability::nova(p, 90, 40, 35, {1, 0.9f, 0.55f, 1});
}
