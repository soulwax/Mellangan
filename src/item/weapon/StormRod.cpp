#include "item/weapon/StormRod.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/Strike.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 28.0f + 12 * Upgrade::at(l, 3); }
float everyAt(int l) { return l >= 4 ? 1.6f : 2.1f; }
int strikesAt(int l) { return 1 + Upgrade::at(l, 2) + Upgrade::at(l, 4) + 2 * Upgrade::at(l, 5); }
float radiusAt(int l) { return 14.0f + 8 * Upgrade::at(l, 3); }
} // namespace

StormRod::StormRod()
    : Upgrade("Storm Rod", atlas::tile(atlas::t::StormStaff),
              {"Calls lightning on random foes", "One more strike",
               "+12 damage, wider blast", "One more strike, faster",
               "Two more strikes"},
              true) {}

void StormRod::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  std::vector<Mob *> pool = p.level->nearestFoes(p, p.x, p.y, 230, 64);
  if (pool.empty())
    return void(cd = 0.2f);
  cd = everyAt(level) * p.haste * pace;
  int n = strikesAt(level);
  float dmg = damageAt(level) * p.might * power, r = radiusAt(level);
  for (int i = 0; i < n; i++) {
    Mob *e = pool[rng.irange(0, int(pool.size()) - 1)];
    p.level->add<Strike>(e->x, e->y, dmg, r)->source = this;
  }
  Sound::thunder.play(0.35f);
}

Figures StormRod::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("strikes", float(strikesAt(l))).add("radius", radiusAt(l));
}
