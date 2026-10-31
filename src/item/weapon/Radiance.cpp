#include "item/weapon/Radiance.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "particle/Mote.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"
#include "level/Level.hpp"

namespace {
constexpr Color HOLY{1, 0.88f, 0.5f, 1};
}

Radiance::Radiance()
    : Upgrade("Radiance", atlas::kit(atlas::kt::Daisies),
              {"A holy nova that mends you per foe", "+6 damage", "Wider nova",
               "Faster novas", "+6 damage, wider nova"},
              true) {
  owner = "PRIESTESS";
}

void Radiance::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 2.4f : 3.2f) * p.haste * pace;
  float r = 55.0f + 12 * (at(level, 3) + at(level, 5)),
        dmg = (14.0f + 6 * (at(level, 2) + at(level, 5))) * p.might * power;
  int hit = 0;
  p.level->foes(p, p.x, p.y, r + 24, [&](Mob &e) {
    double xd = e.x - p.x, yd = e.bodyY() - p.bodyY();
    float rr = r + e.radius;
    if (xd * xd + yd * yd >= rr * rr)
      return;
    e.strike(dmg, xd, yd, 90, this);
    if (hit++ < 8)
      p.level->add<Mote>(e.x, e.bodyY(), atlas::pt::HolyGlint, HOLY,
                         rng.range(0.45f, 0.65f));
  });
  p.hp = std::min(p.maxHp, p.hp + 0.6f * std::min(hit, 10));
  p.level->add<Shockwave>(p.x, p.bodyY(), 6, r, 0.4f, HOLY.alpha(0.85f))
      ->spokes = 12;
  Spark::burst(*p.level, p.x, p.bodyY(), HOLY, 8, r * 1.6f, true);
  Sound::cast.play(0.3f);
}
