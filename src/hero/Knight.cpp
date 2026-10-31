#include "hero/Knight.hpp"

#include "entity/Ability.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {
bool slayer(const Codex &c) { return c.kills() >= 1000; }
} // namespace

Knight::Knight()
    : Hero({"KNIGHT", "Moon Blade", "Sworn shield of the realm",
            "A bulwark halves one blow every 6s", "slay 1000 foes", slayer, 97,
            {0.95f, 0.85f, 0.55f, 1}, 40, 0.92f, 0, 0, 0, 1, 0, 0, "RALLY",
            "A sweeping blow, then a moment of invulnerability", 9,
            {"Moon Blade", "Iron Buckler", "Heartroot"}}) {}

void Knight::cast(Mob &p) const {
  ability::rally(p, 54, 45, 1.5f);
}

void Knight::tick(Mob &p) const {
  p.heroTimer = std::max(0.0f, p.heroTimer - DT);
}

float Knight::guard(Mob &p, float damage) const {
  if (p.heroTimer > 0)
    return damage;
  p.heroTimer = 6;
  Spark::burst(*p.level, p.x, p.bodyY(), col::gold, 10, 80, false);
  return damage * 0.5f;
}
