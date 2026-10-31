#include "hero/Mage.hpp"

#include "entity/Ability.hpp"
#include "entity/Player.hpp"

namespace {
bool always(const Codex &) { return true; }
} // namespace

Mage::Mage()
    : Hero({"MAGE", "Arcane Staff", "Keeper of the old arcana",
            "+15% experience from every gem", "", always, 84,
            {0.78f, 0.55f, 1, 1}, 0, 1, 0, -0.05f, 0, 0, 0, 0, "BLINK",
            "Vanish and reappear ahead in an arcane burst", 6,
            {"Arcane Staff", "Hourglass Tonic", "Warding Halo"}}) {}

void Mage::cast(Mob &p) const {
  ability::blink(p, 90, 35, {0.72f, 0.45f, 1, 1});
}
