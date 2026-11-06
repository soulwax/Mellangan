#include "item/weapon/FireBomb.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Bomb.hpp"
#include "level/Level.hpp"

namespace {
float everyAt(int l) { return l >= 4 ? 2.4f : 3.0f; }
int bombsAt(int l) { return 1 + Upgrade::at(l, 3) + Upgrade::at(l, 5); }
float radiusAt(int l) { return 22.0f + 6 * Upgrade::at(l, 2); }
float fireAt(int l) { return 5.0f + 4 * Upgrade::at(l, 4); }
} // namespace

FireBomb::FireBomb()
    : Upgrade("Fire Bomb", atlas::town(atlas::tt::Bomb),
              {"Lobs a bomb that leaves flames", "Bigger, longer fire",
               "Throws one more bomb", "Hotter flames, faster throws",
               "Throws one more bomb"},
              true) {}

void FireBomb::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = everyAt(level) * p.haste * pace;
  int n = bombsAt(level);
  auto targets = p.level->nearestFoes(p, p.x, p.y, 200, 12);
  for (int i = 0; i < n; i++) {
    double xt, yt;
    if (targets.empty()) {
      double dir = rng.range(0, TAU), r = rng.range(50, 110);
      xt = p.x + std::cos(dir) * r, yt = p.y + std::sin(dir) * r;
    } else {
      Mob *e = targets[rng.irange(0, int(targets.size()) - 1)];
      xt = e->x, yt = e->y;
    }
    p.level
        ->add<Bomb>(p.x, p.bodyY(), xt, yt, 20 * p.might * power,
                    radiusAt(level), fireAt(level) * p.might * power, 2.5f + at(level, 2))
        ->source = this;
  }
  Sound::axe.play(0.4f);
}

Figures FireBomb::figures(int l, bool) const {
  return Figures().add("fire", fireAt(l), Figure::Damage).every(everyAt(l)).add("bombs", float(bombsAt(l))).add("radius", radiusAt(l));
}
