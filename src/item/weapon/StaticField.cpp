#include "item/weapon/StaticField.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Arc.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace {
constexpr Color STATIC{0.5f, 0.78f, 1, 1};
}

StaticField::StaticField()
    : Upgrade("Static Field", atlas::tile(atlas::t::StormStaff),
              {"A crackling field shocks foes close by", "+3 damage",
               "Wider field", "Faster pulses", "+3 damage, wider field"},
              true) {
  owner = "STORMCALLER";
}

void StaticField::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 0.8f : 1.1f) * p.haste * pace;
  float r = 50.0f + 12 * (at(level, 3) + at(level, 5)),
        dmg = (6.0f + 3 * (at(level, 2) + at(level, 5))) * p.might * power;
  int arcs = 0;
  p.level->foes(p, p.x, p.y, r + 24, [&](Mob &e) {
    double xd = e.x - p.x, yd = e.bodyY() - p.bodyY();
    float rr = r + e.radius;
    if (xd * xd + yd * yd >= rr * rr)
      return;
    e.strike(dmg, xd, yd, 15, this);
    if (arcs++ < 6)
      p.level->add<Arc>(std::vector<ArcNode>{{p.x, p.bodyY()},
                                             {e.x, e.bodyY()}},
                        0.16f, STATIC);
  });
  p.level->add<Shockwave>(p.x, p.bodyY(), r * 0.8f, r, 0.3f,
                          STATIC.alpha(0.35f), false);
  Spark::burst(*p.level, p.x, p.bodyY(), STATIC, 4, r * 1.2f, true);
}
