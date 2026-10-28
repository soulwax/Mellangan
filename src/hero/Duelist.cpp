#include "hero/Duelist.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"
#include "entity/projectile/Slash.hpp"

namespace {
bool survivor(const Codex &c) { return c.bestTime >= 600; }

void cut(Player &p) {
  p.level->add<Slash>(p, 1, 30 * p.might, 1.2f);
  p.level->add<Slash>(p, -1, 30 * p.might, 1.2f);
}
} // namespace

Duelist::Duelist()
    : Hero({"DUELIST", "Orbiting Knives", "Blade dancer of the courts",
            "Critical hits strike far harder", "survive 10 minutes", survivor,
            88, {1, 0.45f, 0.7f, 1}, -10, 1.08f, 0, 0, 0, 0, 0, 0.08f,
            "DANCE OF BLADES", "Dash ahead, cutting on both sides", 7,
            {"Orbiting Knives", "Quicksilver", "Lucky Key"}}) {}

void Duelist::cast(Player &p) const {
  cut(p);
  p.x += p.xAim * 110, p.y += p.yAim * 110;
  p.level->collide(p.x, p.y, p.radius);
  p.invuln = std::max(p.invuln, 0.5f);
  cut(p);
  Spark::burst(*p.level, p.x, p.bodyY(), {1, 0.45f, 0.7f, 1}, 14, 120, true);
  Sound::slash.play(0.7f, 0);
}
