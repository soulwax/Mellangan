#include "item/weapon/SoulSiphon.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "particle/Mote.hpp"
#include "particle/Spark.hpp"
#include "level/Level.hpp"

SoulSiphon::SoulSiphon()
    : Upgrade("Soul Siphon", atlas::tile(atlas::t::VialGreen),
              {"Drains the nearest foes to mend you", "Drains one more",
               "+6 damage", "Faster drains", "Drains one more"},
              true) {
  owner = "REAPER";
}

void SoulSiphon::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  auto targets = p.level->nearestFoes(p, p.x, p.y, 160,
                                  3 + at(level, 2) + at(level, 5));
  if (targets.empty())
    return void(cd = 0.3f);
  cd = (level >= 4 ? 2.2f : 3.0f) * p.haste * pace;
  float dmg = (14.0f + 6 * at(level, 3)) * p.might * power;
  for (Mob *e : targets) {
    e->strike(dmg, 0, 0, 0, this);
    Spark::burst(*p.level, e->x, e->bodyY(), {0.9f, 0.3f, 0.35f, 1}, 3, 30,
                 false);
    p.level->add<Mote>(e->x, e->bodyY(), atlas::pt::Wisp,
                       Color{1, 0.55f, 0.62f, 0.95f}, rng.range(0.5f, 0.7f));
  }
  p.hp = std::min(p.maxHp, p.hp + float(targets.size()));
}
