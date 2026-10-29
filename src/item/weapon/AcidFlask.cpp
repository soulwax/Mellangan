#include "item/weapon/AcidFlask.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Bomb.hpp"
#include "level/Level.hpp"

AcidFlask::AcidFlask()
    : Upgrade("Acid Flask", atlas::tile(127),
              {"Hurls flasks that leave burning pools", "Wider pools",
               "Hotter pools", "Throws one more flask", "Longer pools, one more flask"},
              true) {
  owner = "ALCHEMIST";
}

void AcidFlask::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = 2.2f * p.haste * pace;
  auto targets = p.level->nearest(p.x, p.y, 220, 6);
  for (int i = 0, n = 1 + at(level, 4) + at(level, 5); i < n; i++) {
    double xt = p.x + rng.range(-80, 80), yt = p.y + rng.range(-60, 60);
    if (!targets.empty()) {
      Enemy *e = targets[rng.irange(0, int(targets.size()) - 1)];
      xt = e->x, yt = e->y;
    }
    p.level
        ->add<Bomb>(p.x, p.bodyY(), xt, yt, 12 * p.might * power,
                    28.0f + 6 * at(level, 2),
                    (7.0f + 4 * at(level, 3)) * p.might * power,
                    3.5f + at(level, 5), PoolStyle::Acid)
        ->source = this;
  }
  Sound::axe.play(0.3f);
}
