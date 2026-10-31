#include "item/weapon/ChainLightning.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Arc.hpp"
#include "particle/Spark.hpp"

namespace {
constexpr Color STORM{0.55f, 0.75f, 1, 1};
}

ChainLightning::ChainLightning()
    : Upgrade("Chain Lightning", atlas::kit(atlas::kt::Bluebells),
              {"Lightning leaps across three foes", "Leaps to one more",
               "Leaps to one more", "Faster chains", "+8 damage, one more leap"},
              true) {
  owner = "STORMCALLER";
}

void ChainLightning::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  auto targets = p.level->nearestFoes(p, 
      p.x, p.y, 260, 3 + at(level, 2) + at(level, 3) + at(level, 5));
  if (targets.empty())
    return void(cd = 0.2f);
  cd = (level >= 4 ? 1.4f : 1.8f) * p.haste * pace;
  float dmg = (26.0f + 8 * at(level, 5)) * p.might * power;
  std::vector<ArcNode> chain{{p.x, p.bodyY()}};
  while (!targets.empty()) {
    auto next = std::min_element(targets.begin(), targets.end(),
                                 [&](Mob *a, Mob *b) {
                                   auto d = [&](Mob *e) {
                                     double xd = e->x - chain.back().x,
                                            yd = e->bodyY() - chain.back().y;
                                     return xd * xd + yd * yd;
                                   };
                                   return d(a) < d(b);
                                 });
    Mob *e = *next;
    targets.erase(next);
    chain.push_back({e->x, e->bodyY()});
    p.level->foes(p, e->x, e->y, 16 + 24, [&](Mob &o) {
      double xd = o.x - e->x, yd = o.bodyY() - e->y;
      float r = 16 + o.radius;
      if (xd * xd + yd * yd < r * r)
        o.strike(dmg, o.x - e->x, o.y - e->y, 50, this);
    });
    Spark::burst(*p.level, chain.back().x, chain.back().y, STORM, 5, 70, false);
  }
  p.level->add<Arc>(std::move(chain), 0.24f, STORM);
  Sound::thunder.play(0.3f);
}
