#include "item/weapon/AcidFlask.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Bomb.hpp"
#include "level/Level.hpp"

namespace {
int flasksAt(int l) { return 1 + Upgrade::at(l, 4) + Upgrade::at(l, 5); }
float radiusAt(int l) { return 28.0f + 6 * Upgrade::at(l, 2); }
float poolAt(int l) { return 7.0f + 4 * Upgrade::at(l, 3); }
float lastsAt(int l) { return 3.5f + Upgrade::at(l, 5); }
} // namespace

AcidFlask::AcidFlask()
    : Upgrade("Acid Flask", atlas::tile(127),
              {"Hurls flasks that leave burning pools", "Wider pools",
               "Hotter pools", "Throws one more flask", "Longer pools, one more flask"},
              true) {
  owner = "ALCHEMIST";
}

void AcidFlask::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = 2.2f * p.haste * pace;
  auto targets = p.level->nearestFoes(p, p.x, p.y, 220, 6);
  for (int i = 0, n = flasksAt(level); i < n; i++) {
    double xt = p.x + rng.range(-80, 80), yt = p.y + rng.range(-60, 60);
    if (!targets.empty()) {
      Mob *e = targets[rng.irange(0, int(targets.size()) - 1)];
      xt = e->x, yt = e->y;
    }
    p.level
        ->add<Bomb>(p.x, p.bodyY(), xt, yt, 12 * p.might * power,
                    radiusAt(level), poolAt(level) * p.might * power, lastsAt(level),
                    PoolStyle::Acid)
        ->source = this;
  }
  Sound::axe.play(0.3f);
}

Figures AcidFlask::figures(int l, bool) const {
  return Figures().add("pool", poolAt(l), Figure::Damage).add("flasks", float(flasksAt(l))).add("radius", radiusAt(l)).add("lasts", lastsAt(l), Figure::Lasts);
}
