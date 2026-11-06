#include "item/weapon/Meteor.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/MeteorRock.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 50.0f + 20 * Upgrade::at(l, 2); }
float everyAt(int l) { return l >= 4 ? 3.2f : 4.2f; }
float radiusAt(int l) { return 30.0f + 10 * Upgrade::at(l, 3); }
} // namespace

Meteor::Meteor()
    : Upgrade("Meteor", atlas::town(atlas::tt::Bomb),
              {"A meteor crashes into the thickest crowd", "+20 damage",
               "Wider crater", "Falls more often", "Two meteors"},
              true) {
  owner = "MAGE";
}

void Meteor::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  auto targets = p.level->nearestFoes(p, p.x, p.y, 260, 12);
  if (targets.empty())
    return void(cd = 0.3f);
  cd = everyAt(level) * p.haste * pace;
  for (int i = 0; i <= at(level, 5); i++) {
    Mob *e = targets[rng.irange(0, int(targets.size()) - 1)];
    p.level
        ->add<MeteorRock>(e->x, e->y,
                          damageAt(level) * p.might * power, radiusAt(level), 6 * p.might * power)
        ->source = this;
  }
}

Figures Meteor::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("meteors", 1.0f + Upgrade::at(l, 5)).add("radius", radiusAt(l));
}
