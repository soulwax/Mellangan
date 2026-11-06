#include "item/weapon/Judgement.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/Smite.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 38.0f + 12 * Upgrade::at(l, 3); }
float everyAt(int l) { return l >= 4 ? 2.0f : 2.6f; }
int smitesAt(int l) { return 1 + Upgrade::at(l, 2) + Upgrade::at(l, 5); }
float radiusAt(int l) { return 20.0f + 8 * Upgrade::at(l, 3); }
} // namespace

Judgement::Judgement()
    : Upgrade("Judgement", atlas::tile(106),
              {"Smites the nearest foe from above", "Smites one more",
               "+12 damage, wider", "Faster judgement", "Smites one more"},
              true) {
  owner = "KNIGHT";
}

void Judgement::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  auto targets = p.level->nearestFoes(p, p.x, p.y, 220,
                                  smitesAt(level));
  if (targets.empty())
    return void(cd = 0.2f);
  cd = everyAt(level) * p.haste * pace;
  float dmg = damageAt(level) * p.might * power, r = radiusAt(level);
  for (Mob *e : targets)
    p.level->add<Smite>(e->x, e->y, dmg, r)->source = this;
  Sound::thunder.play(0.25f);
}

Figures Judgement::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("smites", float(smitesAt(l))).add("radius", radiusAt(l));
}
