#include "hero/Reaper.hpp"

#include "entity/Player.hpp"

namespace {
bool frenzy(const Codex &c) { return c.bestStreak >= 100; }
} // namespace

Reaper::Reaper()
    : Hero({"REAPER", "Reaper's Scythe", "Harvester of the night",
            "Every kill mends 1.5 health", "a 100 kill streak", frenzy, 86,
            -20, 1, 0.25f, 0, 0, 0, 0, 0}) {}

void Reaper::slay(Player &p, Enemy &) const {
  p.hp = std::min(p.maxHp, p.hp + 1.5f);
}
