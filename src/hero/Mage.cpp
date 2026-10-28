#include "hero/Mage.hpp"

namespace {
bool always(const Codex &) { return true; }
} // namespace

Mage::Mage()
    : Hero({"MAGE", "Arcane Staff", "Keeper of the old arcana",
            "+15% experience from every gem", "", always, 84, 0, 1, 0,
            -0.05f, 0, 0, 0, 0}) {}
