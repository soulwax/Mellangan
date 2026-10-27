#include "meta/Starts.hpp"

namespace {
bool always(const Codex &) { return true; }
bool dawn(const Codex &c) { return c.dawns > 0; }
bool slayer(const Codex &c) { return c.kills() >= 1000; }
bool hoarder(const Codex &c) { return c.count(c.relics) >= 3; }
bool walker(const Codex &c) { return c.count(c.biomes) >= 10; }
bool frenzy(const Codex &c) { return c.bestStreak >= 100; }
} // namespace

const Start STARTS[START_COUNT] = {
    {"Arcane Staff", "", always},
    {"Hunter's Bow", "see a dawn", dawn},
    {"Moon Blade", "slay 1000 foes", slayer},
    {"Fire Bomb", "find 3 relics", hoarder},
    {"Storm Rod", "walk 10 regions", walker},
    {"Reaper's Scythe", "a 100 kill streak", frenzy},
};
