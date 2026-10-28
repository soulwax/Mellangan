#include "hero/Knight.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"
#include "entity/effect/Sweep.hpp"

namespace {
bool slayer(const Codex &c) { return c.kills() >= 1000; }
} // namespace

Knight::Knight()
    : Hero({"KNIGHT", "Moon Blade", "Sworn shield of the realm",
            "A bulwark halves one blow every 6s", "slay 1000 foes", slayer, 97,
            {0.95f, 0.85f, 0.55f, 1}, 40, 0.92f, 0, 0, 0, 1, 0, 0, "RALLY",
            "A sweeping blow, then a moment of invulnerability", 9,
            {"Moon Blade", "Iron Buckler", "Heartroot"}}) {}

void Knight::cast(Player &p) const {
  p.level->add<Sweep>(p, 54, 45 * p.might);
  p.invuln = std::max(p.invuln, 1.5f);
  Spark::burst(*p.level, p.x, p.bodyY(), col::gold, 20, 120, true);
  Sound::scythe.play(0.6f, 0);
}

void Knight::tick(Player &p) const {
  p.heroTimer = std::max(0.0f, p.heroTimer - DT);
}

float Knight::guard(Player &p, float damage) const {
  if (p.heroTimer > 0)
    return damage;
  p.heroTimer = 6;
  Spark::burst(*p.level, p.x, p.bodyY(), col::gold, 10, 80, false);
  return damage * 0.5f;
}
