#include "item/weapon/Bow.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Arrow.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 13.0f + 6 * Upgrade::at(l, 2); }
float everyAt(int l) { return l >= 4 ? 0.6f : 0.85f; }
int arrowsAt(int l) { return 1 + Upgrade::at(l, 3) + Upgrade::at(l, 5); }
int pierceAt(int l) { return 3 + 2 * Upgrade::at(l, 2) + 2 * Upgrade::at(l, 4); }
} // namespace

Bow::Bow()
    : Upgrade("Hunter's Bow", atlas::town(atlas::tt::Bow),
              {"Looses arrows where you walk", "+6 damage, more pierce",
               "Fires a second arrow", "Faster draw, more pierce",
               "Fires a third arrow"},
              true) {}

void Bow::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = everyAt(level) * p.haste * pace;
  int n = arrowsAt(level), pierce = pierceAt(level);
  float dmg = damageAt(level) * p.might * power;
  double aim = std::atan2(p.yAim, p.xAim);
  for (int i = 0; i < n; i++) {
    double dir = aim + (i - (n - 1) * 0.5) * 0.14;
    p.level
        ->add<Arrow>(p.x, p.bodyY(), std::cos(dir) * 320, std::sin(dir) * 320,
                     dmg, pierce)
        ->source = this;
  }
  Sound::bow.play(0.35f);
}

Figures Bow::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("arrows", float(arrowsAt(l))).add("pierce", float(pierceAt(l)));
}
