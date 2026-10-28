#include "item/weapon/Scattershot.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Arrow.hpp"
#include "level/Level.hpp"

Scattershot::Scattershot()
    : Upgrade("Scattershot", atlas::town(atlas::tt::Bow),
              {"Looses arrows in every direction", "+2 arrows", "+6 damage",
               "Faster volleys", "+2 arrows, more pierce"},
              true) {
  owner = "RANGER";
}

void Scattershot::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 1.5f : 2.0f) * p.haste * pace;
  int n = 6 + 2 * (at(level, 2) + at(level, 5));
  float dmg = (12.0f + 6 * at(level, 3)) * p.might * power;
  for (int i = 0; i < n; i++) {
    double dir = TAU * i / n + p.level->time;
    p.level
        ->add<Arrow>(p.x, p.bodyY(), std::cos(dir) * 300, std::sin(dir) * 300,
                     dmg, 2 + 2 * at(level, 5))
        ->source = this;
  }
  Sound::bow.play(0.3f);
}
