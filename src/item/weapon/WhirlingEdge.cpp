#include "item/weapon/WhirlingEdge.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/Sweep.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 14.0f + 6 * (Upgrade::at(l, 2) + Upgrade::at(l, 4)); }
float everyAt(int l) { return l >= 4 ? 1.2f : 1.6f; }
float reachAt(int l) { return 30.0f + 6 * (Upgrade::at(l, 3) + Upgrade::at(l, 5)); }
} // namespace

WhirlingEdge::WhirlingEdge()
    : Upgrade("Whirling Edge", atlas::tile(105),
              {"A blur of steel close around you", "+6 damage", "Longer reach",
               "+6 damage, faster", "Longer reach"},
              true) {
  owner = "DUELIST";
}

void WhirlingEdge::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = everyAt(level) * p.haste * pace;
  p.level
      ->add<Sweep>(p, reachAt(level),
                   damageAt(level) * p.might * power,
                   SweepStyle::Steel)
      ->source = this;
  Sound::slash.play(0.3f);
}

Figures WhirlingEdge::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("reach", reachAt(l));
}
