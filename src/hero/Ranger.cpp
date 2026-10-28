#include "hero/Ranger.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {
bool dawn(const Codex &c) { return c.dawns > 0; }
} // namespace

Ranger::Ranger()
    : Hero({"RANGER", "Hunter's Bow", "Swift hunter of the wilds",
            "Dodges 15% of blows", "see a dawn", dawn, 112, -10, 1.12f, 0,
            0, 8, 0, 0, 0}) {}

float Ranger::guard(Player &p, float damage) const {
  if (!rng.chance(0.15f))
    return damage;
  p.invuln = 0.3f;
  Spark::burst(*p.level, p.x, p.bodyY(), col::white, 6, 70, false);
  return 0;
}
