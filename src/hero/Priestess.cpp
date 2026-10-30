#include "hero/Priestess.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Flourish.hpp"
#include "particle/Spark.hpp"

namespace {
bool devout(const Codex &c) { return c.shrines >= 10; }
} // namespace

Priestess::Priestess()
    : Hero({"PRIESTESS", "Warding Halo", "Lantern of the long night",
            "Shrines kindle 30% faster", "kindle 10 shrines", devout, 99,
            {1, 0.92f, 0.6f, 1}, 10, 1, 0, 0, 0, 0, 0.3f, 0, "SANCTUARY",
            "Heal 35 and drive back the dark", 14,
            {"Warding Halo", "Heartroot", "Troll Blood"}}) {}

void Priestess::cast(Player &p) const {
  p.heal(35);
  p.level->enemiesNear(p.x, p.y, 110, [&](Enemy &e) {
    double xd = e.x - p.x, yd = e.y - p.y;
    if (xd * xd + yd * yd < 90 * 90)
      e.hurt(40 * p.might, xd, yd, 220);
  });
  constexpr Color HOLY{1, 0.9f, 0.55f, 1};
  flourish::sigil(*p.level, p.x, p.bodyY(), 90, HOLY, 16);
  flourish::scatter(*p.level, p.x, p.bodyY(), atlas::pt::HolyGlint, HOLY, 12,
                    40, -50);
  Spark::burst(*p.level, p.x, p.bodyY(), HOLY, 12, 170, true);
  Sound::nova.play(0.5f, 0);
}
