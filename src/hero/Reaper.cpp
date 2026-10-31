#include "hero/Reaper.hpp"

#include "entity/Ability.hpp"

#include "entity/Player.hpp"

namespace {
bool frenzy(const Codex &c) { return c.bestStreak >= 100; }
} // namespace

Reaper::Reaper()
    : Hero({"REAPER", "Reaper's Scythe", "Harvester of the night",
            "Every kill mends 1.5 health", "a 100 kill streak", frenzy, 86,
            {0.9f, 0.3f, 0.35f, 1}, -20, 1, 0.25f, 0, 0, 0, 0, 0,
            "DEATH'S EMBRACE", "A vast reaping arc that drinks life", 10,
            {"Reaper's Scythe", "Ogre Hammer", "Heartroot"}}) {}

void Reaper::cast(Mob &p) const {
  ability::reap(p, 82, 60);
}

void Reaper::slay(Mob &p, Enemy &) const {
  p.hp = std::min(p.maxHp, p.hp + 1.5f);
}
