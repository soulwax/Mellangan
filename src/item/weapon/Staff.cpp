#include "item/weapon/Staff.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Bolt.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {
float damageAt(int l) { return 10.0f + 8 * Upgrade::at(l, 2) + 8 * Upgrade::at(l, 4); }
float everyAt(int l) { return l >= 2 ? 0.85f : 1.05f; }
int boltsAt(int l) { return 1 + Upgrade::at(l, 3) + Upgrade::at(l, 5); }
} // namespace

Staff::Staff()
    : Upgrade("Arcane Staff", atlas::tile(atlas::t::Staff),
              {"Fires bolts at the nearest foe", "+8 damage, casts faster",
               "Fires one more bolt", "+8 damage, bolts pierce one more foe",
               "Fires one more bolt"},
              true) {}

void Staff::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  int n = boltsAt(level);
  auto targets = p.level->nearestFoes(p, p.x, p.y, 250, n);
  if (targets.empty())
    return void(cd = 0);
  cd = everyAt(level) * p.haste * pace;
  for (int i = 0; i < n; i++) {
    Mob *e = targets[i % targets.size()];
    float spread = i < int(targets.size()) ? 0 : rng.range(-0.35f, 0.35f);
    double dir = std::atan2(e->bodyY() - p.bodyY(), e->x - p.x) + spread;
    p.level
        ->add<Bolt>(p.x, p.bodyY(), std::cos(dir) * 230, std::sin(dir) * 230,
                    damageAt(level) * p.might * power, 1 + at(level, 4),
                    e->id)
        ->source = this;
  }
  Spark::burst(*p.level, p.x, p.bodyY(), {0.72f, 0.45f, 1, 1}, 4, 40, true);
  Sound::cast.play(0.3f, 0.1f);
}

Figures Staff::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("bolts", float(boltsAt(l))).add("pierce", 1.0f + Upgrade::at(l, 4));
}
