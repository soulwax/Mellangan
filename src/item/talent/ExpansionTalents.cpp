#include "item/talent/ExpansionTalents.hpp"

#include <algorithm>

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/projectile/Arrow.hpp"
#include "level/Level.hpp"
#include "particle/Flourish.hpp"
#include "particle/Mark.hpp"
#include "particle/Spark.hpp"

namespace {
struct TalentDef {
  const char *name, *owner, *desc;
  int icon;
};

const TalentDef TALENTS[] = {
    {"Heartwood", "WARDEN", "Skills heal 4 per rank and root nearby foes",
     atlas::kt::Ivy},
    {"Shrapnel", "TINKERER", "Slain foes may burst into shrapnel",
     atlas::kt::RockPile},
    {"Starsight", "ORACLE", "Every 6s, foresee and mark 2 foes per rank",
     atlas::kt::Bluebells},
    {"Plunder", "CORSAIR", "Kills may spill an extra gem", atlas::kt::Pampas},
};
} // namespace

ExpansionTalent::ExpansionTalent(int kind)
    : Upgrade(TALENTS[std::clamp(kind, 0, 3)].name,
              atlas::kit(TALENTS[std::clamp(kind, 0, 3)].icon), {}, false),
      kind(std::clamp(kind, 0, 3)) {
  const TalentDef &d = TALENTS[this->kind];
  owner = d.owner;
  desc = {d.desc, d.desc, d.desc, d.desc, d.desc};
}

void ExpansionTalent::cast(Mob &p) {
  if (kind != 0)
    return;
  p.mend(4.0f * level);
  float reach = 70 + 10.0f * level;
  p.level->foes(p, p.x, p.y, reach + 20, [&](Mob &e) {
    double xd = e.x - p.x, yd = e.y - p.y;
    if (xd * xd + yd * yd < reach * reach)
      e.afflict(StatusKind::Chill, 1.5f, 0.65f, this);
  });
  flourish::scatter(*p.level, p.x, p.y, atlas::pt::Leaf, Color{0.45f, 0.85f, 0.35f, 1},
                    8, reach, -10);
}

void ExpansionTalent::slay(Mob &p, Enemy &e) {
  if (kind == 1 && rng.chance(0.05f * level)) {
    for (int i = 0; i < 6; i++) {
      double a = TAU * i / 6 + rng.range(0, 0.5f);
      p.level->add<Arrow>(e.x, e.bodyY(), std::cos(a) * 220, std::sin(a) * 220,
                          10 * p.might, 1, ArrowStyle::Scatter)
          ->hostile = p.hostile();
    }
    Spark::burst(*p.level, e.x, e.bodyY(), Color{1, 0.75f, 0.4f, 1}, 6, 90, true);
    Sound::axe.play(0.2f, 0.06f, 1.5f);
  } else if (kind == 3 && rng.chance(0.04f * level)) {
    Gem::drop(*p.level, e.x + rng.range(-6, 6), e.y + rng.range(-4, 4),
              5 * p.level->xpMul);
    Spark::burst(*p.level, e.x, e.bodyY(), col::gold, 3, 40, false);
  }
}

void ExpansionTalent::tick(Mob &p) {
  if (kind != 2 || (timer -= DT) > 0)
    return;
  timer = 6;
  for (Mob *e : p.level->nearestFoes(p, p.x, p.y, 220, 2 * level)) {
    e->afflict(StatusKind::Marked, 4, 0.25f, this);
    p.level->add<Mark>(e->x, e->bodyY() - 12, 1.2f, atlas::pt::ArcaneStar,
                       Color{0.85f, 0.7f, 1, 1}, 1, 0);
  }
}
