#include "item/weapon/Scythe.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/Sweep.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 24.0f + 10 * (Upgrade::at(l, 2) + Upgrade::at(l, 4) + Upgrade::at(l, 5)); }
float everyAt(int l) { return l >= 4 ? 2.0f : 2.8f; }
float reachAt(int l) { return 28.0f + 8 * (Upgrade::at(l, 3) + Upgrade::at(l, 5)); }
} // namespace

Scythe::Scythe()
    : Upgrade("Reaper's Scythe", atlas::town(atlas::tt::Scythe),
              {"Sweeps a full circle around you", "+10 damage", "Longer reach",
               "+10 damage, faster sweeps", "Longer reach, +10 damage"},
              true) {}

void Scythe::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = everyAt(level) * p.haste * pace;
  p.level
      ->add<Sweep>(
          p, reachAt(level),
          damageAt(level) * p.might * power)
      ->source = this;
  Sound::scythe.play(0.5f);
}

Figures Scythe::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("reach", reachAt(l));
}
