#include "item/weapon/PiercingShot.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Arrow.hpp"
#include "level/Level.hpp"

PiercingShot::PiercingShot()
    : Upgrade("Piercing Shot", atlas::town(atlas::tt::Arrow),
              {"A heavy arrow through the nearest foe", "+10 damage",
               "Pierces 4 more", "+10 damage, faster draw", "Looses a second shot"},
              true) {
  owner = "RANGER";
}

void PiercingShot::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  auto targets = p.level->nearest(p.x, p.y, 300, 1);
  if (targets.empty())
    return void(cd = 0.2f);
  cd = (level >= 4 ? 1.2f : 1.6f) * p.haste * pace;
  Enemy *e = targets.front();
  float dmg = (30.0f + 10 * (at(level, 2) + at(level, 4))) * p.might * power;
  for (int i = 0; i <= at(level, 5); i++) {
    double dir = std::atan2(e->bodyY() - p.bodyY(), e->x - p.x) + i * 0.08;
    p.level
        ->add<Arrow>(p.x, p.bodyY(), std::cos(dir) * 420, std::sin(dir) * 420,
                     dmg, 8 + 4 * at(level, 3))
        ->source = this;
  }
  Sound::bow.play(0.4f);
}
