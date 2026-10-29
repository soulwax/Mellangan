#include "item/weapon/Meteor.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/MeteorRock.hpp"
#include "level/Level.hpp"

Meteor::Meteor()
    : Upgrade("Meteor", atlas::town(atlas::tt::Bomb),
              {"A meteor crashes into the thickest crowd", "+20 damage",
               "Wider crater", "Falls more often", "Two meteors"},
              true) {
  owner = "MAGE";
}

void Meteor::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  auto targets = p.level->nearest(p.x, p.y, 260, 12);
  if (targets.empty())
    return void(cd = 0.3f);
  cd = (level >= 4 ? 3.2f : 4.2f) * p.haste * pace;
  for (int i = 0; i <= at(level, 5); i++) {
    Enemy *e = targets[rng.irange(0, int(targets.size()) - 1)];
    p.level
        ->add<MeteorRock>(e->x, e->y,
                          (50.0f + 20 * at(level, 2)) * p.might * power,
                          30.0f + 10 * at(level, 3), 6 * p.might * power)
        ->source = this;
  }
}
