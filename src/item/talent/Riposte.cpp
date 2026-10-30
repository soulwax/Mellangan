#include "item/talent/Riposte.hpp"

#include "entity/Player.hpp"
#include "entity/effect/Sweep.hpp"
#include "level/Level.hpp"

Riposte::Riposte()
    : Upgrade("Riposte", atlas::kit(atlas::kt::Thornbush),
              {"Chance to parry a blow and cut back", "Chance to parry a blow and cut back", "Chance to parry a blow and cut back", "Chance to parry a blow and cut back", "Chance to parry a blow and cut back"},
              false) {
  owner = "DUELIST";
}

float Riposte::guard(Player &p, float damage) {
  if (!rng.chance(0.08f * level))
    return damage;
  p.invuln = std::max(p.invuln, 0.4f);
  p.level->add<Sweep>(p, 40, 30 * p.might, SweepStyle::Steel)->source = this;
  return 0;
}
