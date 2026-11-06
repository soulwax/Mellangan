#include "item/weapon/Glaive.hpp"

#include <cmath>

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/GlaiveThrust.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 24.0f + 12 * Upgrade::at(l, 2) + 14 * Upgrade::at(l, 4); }
float everyAt(int l) { return l >= 4 ? 1.1f : 1.45f; }
float reachAt(int l) { return 48.0f * (1.0f + 0.25f * Upgrade::at(l, 3)); }
} // namespace

Glaive::Glaive()
    : Upgrade("Glaive", atlas::frontier(atlas::fr::Glaive),
              {"Thrusts a broad blade forward in a line", "+12 damage",
               "+25% reach and knockback", "+14 damage, faster thrusts",
               "Thrusts strike both forward and behind"},
              true) {}

void Glaive::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = everyAt(level) * p.haste * pace;
  Sound::slash.play(0.45f, -0.05f);

  float dmg = damageAt(level) * p.might * power;
  float reach = reachAt(level);
  float force = 140.0f * (1.0f + 0.25f * at(level, 3));
  bool doubleEnded = at(level, 5) > 0;

  double dirX = p.xAim, dirY = p.yAim;
  if (std::abs(dirX) < 1e-4 && std::abs(dirY) < 1e-4)
    dirX = p.facing, dirY = 0;

  p.level
      ->add<GlaiveThrust>(p, dirX, dirY, reach, dmg, force, doubleEnded)
      ->source = this;
}

Figures Glaive::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("reach", reachAt(l)).add("thrusts", 1.0f + Upgrade::at(l, 5));
}
