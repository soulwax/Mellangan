#include "item/weapon/FireBomb.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Bomb.hpp"
#include "level/Level.hpp"

FireBomb::FireBomb()
    : Upgrade("Fire Bomb", atlas::town(atlas::tt::Bomb),
              {"Lobs a bomb that leaves flames", "Bigger, longer fire",
               "Throws one more bomb", "Hotter flames, faster throws",
               "Throws one more bomb"},
              true) {}

void FireBomb::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 2.4f : 3.0f) * p.haste * pace;
  int n = 1 + at(level, 3) + at(level, 5);
  auto targets = p.level->nearest(p.x, p.y, 200, 12);
  for (int i = 0; i < n; i++) {
    double xt, yt;
    if (targets.empty()) {
      double dir = rng.range(0, TAU), r = rng.range(50, 110);
      xt = p.x + std::cos(dir) * r, yt = p.y + std::sin(dir) * r;
    } else {
      Enemy *e = targets[rng.irange(0, int(targets.size()) - 1)];
      xt = e->x, yt = e->y;
    }
    p.level
        ->add<Bomb>(p.x, p.bodyY(), xt, yt, 20 * p.might * power,
                    22.0f + 6 * at(level, 2),
                    (5.0f + 4 * at(level, 4)) * p.might * power, 2.5f + at(level, 2))
        ->source = this;
  }
  Sound::axe.play(0.4f);
}
