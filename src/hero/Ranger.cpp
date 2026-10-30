#include "hero/Ranger.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Drift.hpp"
#include "particle/Spark.hpp"
#include "entity/projectile/Arrow.hpp"

namespace {
bool dawn(const Codex &c) { return c.dawns > 0; }
} // namespace

Ranger::Ranger()
    : Hero({"RANGER", "Hunter's Bow", "Swift hunter of the wilds",
            "Dodges 15% of blows", "see a dawn", dawn, 112,
            {0.55f, 0.9f, 0.45f, 1}, -10, 1.12f, 0, 0, 8, 0, 0, 0, "VOLLEY",
            "Loose a fan of eleven arrows", 5,
            {"Hunter's Bow", "Quicksilver", "Lucky Key"}}) {}

void Ranger::cast(Player &p) const {
  double aim = std::atan2(p.yAim, p.xAim);
  for (int i = 0; i < 11; i++) {
    double dir = aim + (i - 5) * 0.09;
    p.level->add<Arrow>(p.x, p.bodyY(), std::cos(dir) * 360,
                        std::sin(dir) * 360, 18 * p.might, 3);
    if (i % 2 == 0 && Drift::room()) {
      Drift *d = p.level->add<Drift>(p.x, p.bodyY(), std::cos(dir) * 110,
                                     std::sin(dir) * 110, atlas::pt::Leaf,
                                     col::white, rng.range(0.5f, 0.8f));
      d->spin = rng.range(-4, 4), d->sway = 10, d->gravity = 20;
    }
  }
  p.level->add<Shockwave>(p.x, p.bodyY(), 8, 40, 0.3f,
                          Color{0.6f, 0.95f, 0.5f, 0.7f}, false);
  Sound::bow.play(0.7f, 0);
}

float Ranger::guard(Player &p, float damage) const {
  if (!rng.chance(0.15f))
    return damage;
  p.invuln = 0.3f;
  Spark::burst(*p.level, p.x, p.bodyY(), col::white, 6, 70, false);
  return 0;
}
