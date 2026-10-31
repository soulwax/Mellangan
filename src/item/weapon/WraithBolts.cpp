#include "item/weapon/WraithBolts.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Bolt.hpp"
#include "level/Level.hpp"

WraithBolts::WraithBolts()
    : Upgrade("Wraith Bolts", atlas::kit(atlas::kt::Skull),
              {"Restless souls hunt the nearest foes", "One more wraith",
               "+6 damage", "Wraiths pierce, faster", "One more wraith"},
              true) {
  owner = "REAPER";
}

void WraithBolts::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  auto targets = p.level->nearestFoes(p, p.x, p.y, 250,
                                  3 + at(level, 2) + at(level, 5));
  if (targets.empty())
    return void(cd = 0.2f);
  cd = (level >= 4 ? 1.8f : 2.4f) * p.haste * pace;
  float dmg = (16.0f + 6 * at(level, 3)) * p.might * power;
  for (Mob *e : targets) {
    double dir = std::atan2(e->bodyY() - p.bodyY(), e->x - p.x);
    p.level
        ->add<Bolt>(p.x, p.bodyY(), std::cos(dir) * 200, std::sin(dir) * 200,
                    dmg, 1 + at(level, 4), e->id, BoltStyle::Wraith)
        ->source = this;
  }
  Sound::cast.play(0.3f);
}
