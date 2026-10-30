#include "item/weapon/WhirlingEdge.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/Sweep.hpp"
#include "level/Level.hpp"

WhirlingEdge::WhirlingEdge()
    : Upgrade("Whirling Edge", atlas::tile(105),
              {"A blur of steel close around you", "+6 damage", "Longer reach",
               "+6 damage, faster", "Longer reach"},
              true) {
  owner = "DUELIST";
}

void WhirlingEdge::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 1.2f : 1.6f) * p.haste * pace;
  p.level
      ->add<Sweep>(p, 30.0f + 6 * (at(level, 3) + at(level, 5)),
                   (14.0f + 6 * (at(level, 2) + at(level, 4))) * p.might *
                       power,
                   SweepStyle::Steel)
      ->source = this;
  Sound::slash.play(0.3f);
}
