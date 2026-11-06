#include "item/weapon/Censer.hpp"

#include <cmath>

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/IncenseCloud.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 8.0f + 4 * Upgrade::at(l, 2) + 6 * Upgrade::at(l, 4); }
int cloudsAt(int l) { return 1 + Upgrade::at(l, 3) + 2 * Upgrade::at(l, 5); }
float lastsAt(int l) { return 2.4f + 0.8f * Upgrade::at(l, 4); }
} // namespace

Censer::Censer()
    : Upgrade("Censer", atlas::frontier(atlas::fr::Censer),
              {"Releases searing holy incense clouds", "+4 damage per pulse",
               "+1 incense cloud", "+6 damage, longer clouds",
               "+2 incense clouds"},
              true) {}

void Censer::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = 2.2f * p.haste * pace;
  Sound::orb.play(0.35f, 0.2f);

  int count = cloudsAt(level);
  float dmg = damageAt(level) * p.might * power;
  float duration = lastsAt(level);
  float rad = 26.0f;

  for (int i = 0; i < count; i++) {
    double a = rng.range(0, TAU);
    double dist = rng.range(12, 32);
    double vx = std::cos(a) * rng.range(15, 35);
    double vy = std::sin(a) * rng.range(15, 35);
    p.level
        ->add<IncenseCloud>(p.x + std::cos(a) * dist,
                            p.bodyY() + std::sin(a) * dist, vx, vy, rad, dmg,
                            duration)
        ->source = this;
  }
}

Figures Censer::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(2.2f).add("clouds", float(cloudsAt(l))).add("lasts", lastsAt(l), Figure::Lasts);
}
