#include "hero/Duelist.hpp"

#include "entity/Ability.hpp"

#include "entity/Player.hpp"

namespace {
bool survivor(const Codex &c) { return c.bestTime >= 600; }

} // namespace

Duelist::Duelist()
    : Hero({"DUELIST", "Orbiting Knives", "Blade dancer of the courts",
            "Critical hits strike far harder", "survive 10 minutes", survivor,
            88, {1, 0.45f, 0.7f, 1}, -10, 1.08f, 0, 0, 0, 0, 0, 0.08f,
            "DANCE OF BLADES", "Dash ahead, cutting on both sides", 7,
            {"Orbiting Knives", "Quicksilver", "Lucky Key"}}) {}

void Duelist::cast(Mob &p) const {
  ability::dance(p, 110, 30, {1, 0.45f, 0.7f, 1});
}
