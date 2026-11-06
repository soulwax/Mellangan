#include "item/weapon/WraithBolts.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Bolt.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 36.0f + 14 * Upgrade::at(l, 3); }
float everyAt(int l) { return l >= 4 ? 1.8f : 2.4f; }
int wraithsAt(int l) { return 3 + Upgrade::at(l, 2) + Upgrade::at(l, 5); }
} // namespace

WraithBolts::WraithBolts()
    : Upgrade("Wraith Bolts", atlas::kit(atlas::kt::Skull),
              {"Restless souls hunt the nearest foes", "One more wraith",
               "+14 damage", "Wraiths pierce, faster", "One more wraith"},
              true) {
  owner = "REAPER";
}

void WraithBolts::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  auto targets = p.level->nearestFoes(p, p.x, p.y, 250,
                                  wraithsAt(level));
  if (targets.empty())
    return void(cd = 0.2f);
  cd = everyAt(level) * p.haste * pace;
  float dmg = damageAt(level) * p.might * power;
  for (Mob *e : targets) {
    double dir = std::atan2(e->bodyY() - p.bodyY(), e->x - p.x);
    p.level
        ->add<Bolt>(p.x, p.bodyY(), std::cos(dir) * 200, std::sin(dir) * 200,
                    dmg, 1 + at(level, 4), e->id, BoltStyle::Wraith)
        ->source = this;
  }
  Sound::cast.play(0.3f);
}

Figures WraithBolts::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("wraiths", float(wraithsAt(l))).add("pierce", 1.0f + Upgrade::at(l, 4));
}
