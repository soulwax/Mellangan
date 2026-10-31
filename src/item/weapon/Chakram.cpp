#include "item/weapon/Chakram.hpp"

#include <cmath>

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/ChakramDisc.hpp"
#include "level/Level.hpp"

Chakram::Chakram()
    : Upgrade("Chakram", atlas::frontier(atlas::fr::Chakram),
              {"Hurls a returning bladed disc", "+8 damage",
               "+1 chakram", "+10 damage, faster return", "+1 chakram"},
              true) {}

void Chakram::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = 2.0f * p.haste * pace;
  Sound::slash.play(0.35f, 0.08f);

  int count = 1 + at(level, 3) + at(level, 5);
  float dmg = (18.0f + 8 * at(level, 2) + 10 * at(level, 4)) * p.might * power;
  double aimAngle = std::atan2(p.yAim, p.xAim);
  if (std::abs(p.xAim) < 1e-4 && std::abs(p.yAim) < 1e-4)
    aimAngle = p.facing > 0 ? 0 : 3.14159;

  for (int i = 0; i < count; i++) {
    double spread = (i - (count - 1) * 0.5) * 0.28;
    double a = aimAngle + spread;
    double spd = 260.0;
    p.level
        ->add<ChakramDisc>(p.x, p.bodyY(), std::cos(a) * spd,
                           std::sin(a) * spd, dmg, 12)
        ->source = this;
  }
}

