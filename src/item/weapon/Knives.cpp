#include "item/weapon/Knives.hpp"

#include "entity/Player.hpp"
#include "entity/effect/Knife.hpp"
#include "level/Level.hpp"

Knives::Knives()
    : Upgrade("Orbiting Knives", atlas::tile(atlas::t::Dagger),
              {"Two knives circle around you", "One more knife",
               "One more knife, +4 damage", "One more knife",
               "Faster spin, +4 damage"},
              true) {}

void Knives::levelUp(Player &p) {
  Upgrade::levelUp(p);
  while (spawned < count())
    p.level->add<Knife>(*this, p, spawned++)->source = this;
}

void Knives::tick(Player &) { angle += (level >= 5 ? 4.6f : 3.4f) * DT; }
