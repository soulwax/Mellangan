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

namespace {
float damageAt(int l) { return 6.0f + 3 * (Upgrade::at(l, 2) + Upgrade::at(l, 5)); }
float everyAt(int l) { return l >= 4 ? 0.8f : 1.1f; }
float radiusAt(int l) { return 50.0f + 12 * (Upgrade::at(l, 3) + Upgrade::at(l, 5)); }
} // namespace

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
  cd = everyAt(level) * p.haste * pace;
  float r = radiusAt(level), dmg = damageAt(level) * p.might * power * p.arcs;
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
  if (arcs)
    Sound::thunder.play(0.1f, 0.1f, 1.8f);
  p.level->add<Shockwave>(p.x, p.bodyY(), r * 0.8f, r, 0.3f,
                          STATIC.alpha(0.35f), false);
  Spark::burst(*p.level, p.x, p.bodyY(), STATIC, 4, r * 1.2f, true);
}

Figures StaticField::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("radius", radiusAt(l));
}
