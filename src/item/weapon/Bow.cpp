#include "item/weapon/Bow.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Arrow.hpp"
#include "level/Level.hpp"

Bow::Bow()
    : Upgrade("Hunter's Bow", atlas::town(atlas::tt::Bow),
              {"Looses arrows where you walk", "+6 damage, more pierce",
               "Fires a second arrow", "Faster draw, more pierce",
               "Fires a third arrow"},
              true) {}

void Bow::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 0.6f : 0.85f) * p.haste;
  int n = 1 + at(level, 3) + at(level, 5),
      pierce = 3 + 2 * at(level, 2) + 2 * at(level, 4);
  float dmg = (13.0f + 6 * at(level, 2)) * p.might;
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
