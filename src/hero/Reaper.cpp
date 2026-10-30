#include "hero/Reaper.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Mote.hpp"
#include "particle/Flourish.hpp"
#include "particle/Spark.hpp"
#include "entity/effect/Sweep.hpp"

namespace {
bool frenzy(const Codex &c) { return c.bestStreak >= 100; }
} // namespace

Reaper::Reaper()
    : Hero({"REAPER", "Reaper's Scythe", "Harvester of the night",
            "Every kill mends 1.5 health", "a 100 kill streak", frenzy, 86,
            {0.9f, 0.3f, 0.35f, 1}, -20, 1, 0.25f, 0, 0, 0, 0, 0,
            "DEATH'S EMBRACE", "A vast reaping arc that drinks life", 10,
            {"Reaper's Scythe", "Ogre Hammer", "Heartroot"}}) {}

void Reaper::cast(Player &p) const {
  p.level->add<Sweep>(p, 82, 60 * p.might);
  int caught = 0;
  p.level->enemiesNear(p.x, p.y, 90, [&](Enemy &e) {
    if (caught++ < 10)
      p.level->add<Mote>(e.x, e.bodyY(), atlas::pt::Wisp,
                         Color{1, 0.55f, 0.62f, 0.95f}, rng.range(0.5f, 0.8f));
  });
  p.hp = std::min(p.maxHp, p.hp + 2.0f * caught);
  flourish::sigil(*p.level, p.x, p.bodyY(), 82, Color{0.9f, 0.25f, 0.32f, 1}, 0);
  flourish::scatter(*p.level, p.x, p.bodyY(), atlas::pt::Puff,
                    Color{0.22f, 0.08f, 0.14f, 0.6f}, 8, 70);
  Spark::burst(*p.level, p.x, p.bodyY(), {0.9f, 0.3f, 0.35f, 1}, 10, 150, true);
  Sound::scythe.play(0.7f, 0);
}

void Reaper::slay(Player &p, Enemy &) const {
  p.hp = std::min(p.maxHp, p.hp + 1.5f);
}
