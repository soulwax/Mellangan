#include "item/weapon/AxeThrow.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Axe.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 26.0f + 10 * (Upgrade::at(l, 2) + Upgrade::at(l, 4)); }
int axesAt(int l) { return 1 + Upgrade::at(l, 3) + Upgrade::at(l, 5); }
int pierceAt(int l) { return 5 + 4 * Upgrade::at(l, 4); }
} // namespace

AxeThrow::AxeThrow()
    : Upgrade("Hurled Axe", atlas::tile(atlas::t::Axe),
              {"Lobs a heavy spinning axe", "+10 damage", "Throws one more axe",
               "+10 damage, more pierce", "Throws one more axe"},
              true) {}

void AxeThrow::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = 2.3f * p.haste * pace;
  Sound::axe.play(0.45f);
  int n = axesAt(level);
  float dmg = damageAt(level) * p.might * power;
  for (int i = 0; i < n; i++) {
    double xa = p.facing * rng.range(25, 70) + (i - (n - 1) * 0.5f) * 55,
           ya = -rng.range(230, 260);
    p.level
        ->add<Axe>(p.x, p.bodyY(), xa, ya, dmg, pierceAt(level),
                   p.facing * 14)
        ->source = this;
  }
}

Figures AxeThrow::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(2.3f).add("axes", float(axesAt(l))).add("pierce", float(pierceAt(l)));
}
