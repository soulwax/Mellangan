#include "item/weapon/Scythe.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/Sweep.hpp"
#include "level/Level.hpp"

Scythe::Scythe()
    : Upgrade("Reaper's Scythe", atlas::town(atlas::tt::Scythe),
              {"Sweeps a full circle around you", "+10 damage", "Longer reach",
               "+10 damage, faster sweeps", "Longer reach, +10 damage"},
              true) {}

void Scythe::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 2.0f : 2.8f) * p.haste;
  p.level
      ->add<Sweep>(
          p, 28.0f + 8 * (at(level, 3) + at(level, 5)),
          (24.0f + 10 * (at(level, 2) + at(level, 4) + at(level, 5))) * p.might)
      ->source = this;
  Sound::scythe.play(0.5f);
}
