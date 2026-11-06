#include "item/weapon/PiercingShot.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Arrow.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 30.0f + 10 * (Upgrade::at(l, 2) + Upgrade::at(l, 4)); }
float everyAt(int l) { return l >= 4 ? 1.2f : 1.6f; }
int pierceAt(int l) { return 8 + 4 * Upgrade::at(l, 3); }
} // namespace

PiercingShot::PiercingShot()
    : Upgrade("Piercing Shot", atlas::town(atlas::tt::Arrow),
              {"A heavy arrow through the nearest foe", "+10 damage",
               "Pierces 4 more", "+10 damage, faster draw", "Looses a second shot"},
              true) {
  owner = "RANGER";
}

void PiercingShot::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  auto targets = p.level->nearestFoes(p, p.x, p.y, 300, 1);
  if (targets.empty())
    return void(cd = 0.2f);
  cd = everyAt(level) * p.haste * pace;
  Mob *e = targets.front();
  float dmg = damageAt(level) * p.might * power;
  for (int i = 0; i <= at(level, 5); i++) {
    double dir = std::atan2(e->bodyY() - p.bodyY(), e->x - p.x) + i * 0.08;
    p.level
        ->add<Arrow>(p.x, p.bodyY(), std::cos(dir) * 420, std::sin(dir) * 420,
                     dmg, pierceAt(level), ArrowStyle::Piercing)
        ->source = this;
  }
  Sound::bow.play(0.4f);
}

Figures PiercingShot::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("shots", 1.0f + Upgrade::at(l, 5)).add("pierce", float(pierceAt(l)));
}
