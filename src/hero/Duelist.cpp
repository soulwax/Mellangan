#include "hero/Duelist.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Flourish.hpp"
#include "particle/Spark.hpp"
#include "entity/projectile/Slash.hpp"

namespace {
bool survivor(const Codex &c) { return c.bestTime >= 600; }

void cut(Player &p) {
  p.level->add<Slash>(p, 1, 30 * p.might, 1.2f, SlashStyle::Flurry);
  p.level->add<Slash>(p, -1, 30 * p.might, 1.2f, SlashStyle::Flurry);
}
} // namespace

Duelist::Duelist()
    : Hero({"DUELIST", "Orbiting Knives", "Blade dancer of the courts",
            "Critical hits strike far harder", "survive 10 minutes", survivor,
            88, {1, 0.45f, 0.7f, 1}, -10, 1.08f, 0, 0, 0, 0, 0, 0.08f,
            "DANCE OF BLADES", "Dash ahead, cutting on both sides", 7,
            {"Orbiting Knives", "Quicksilver", "Lucky Key"}}) {}

void Duelist::cast(Player &p) const {
  double x0 = p.x, y0 = p.bodyY();
  cut(p);
  flourish::echo(*p.level, p.x, p.y, sprite(), Color{1, 0.6f, 0.8f, 1});
  p.x += p.xAim * 110, p.y += p.yAim * 110;
  p.level->collide(p.x, p.y, p.radius);
  p.invuln = std::max(p.invuln, 0.5f);
  flourish::streak(*p.level, x0, y0, p.x, p.bodyY(), atlas::pt::Petal,
                   col::white, 9);
  cut(p);
  Spark::burst(*p.level, p.x, p.bodyY(), {1, 0.45f, 0.7f, 1}, 14, 120, true);
  Sound::slash.play(0.7f, 0);
}
