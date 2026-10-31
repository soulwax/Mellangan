#include "hero/Stormcaller.hpp"

#include "entity/Ability.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Flourish.hpp"

namespace {
bool walker(const Codex &c) { return c.count(c.biomes) >= 10; }
} // namespace

Stormcaller::Stormcaller()
    : Hero({"STORMCALLER", "Storm Rod", "Voice of the thunderheads",
            "Every 20th kill calls down lightning", "walk 10 regions", walker,
            87, {0.5f, 0.78f, 1, 1}, 0, 1, 0, -0.1f, 0, 0, 0, 0.06f,
            "THUNDERCLAP", "Six bolts crash down around you", 8,
            {"Storm Rod", "Lucky Key", "Arcane Staff"}}) {}

void Stormcaller::cast(Mob &p) const {
  ability::thunder(p, 6, 50);
  flourish::sigil(*p.level, p.x, p.bodyY(), 90, Color{0.6f, 0.82f, 1, 1}, 6);
  p.level->game.onExplosion();
}

void Stormcaller::slay(Mob &p, Enemy &) const {
  if (++p.heroCount % 20 == 0)
    ability::thunder(p, 3, 40);
}
