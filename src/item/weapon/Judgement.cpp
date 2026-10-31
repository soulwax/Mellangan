#include "item/weapon/Judgement.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/Smite.hpp"
#include "level/Level.hpp"

Judgement::Judgement()
    : Upgrade("Judgement", atlas::tile(106),
              {"Smites the nearest foe from above", "Smites one more",
               "+12 damage, wider", "Faster judgement", "Smites one more"},
              true) {
  owner = "KNIGHT";
}

void Judgement::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  auto targets = p.level->nearestFoes(p, p.x, p.y, 220,
                                  1 + at(level, 2) + at(level, 5));
  if (targets.empty())
    return void(cd = 0.2f);
  cd = (level >= 4 ? 2.0f : 2.6f) * p.haste * pace;
  float dmg = (38.0f + 12 * at(level, 3)) * p.might * power,
        r = 20.0f + 8 * at(level, 3);
  for (Mob *e : targets)
    p.level->add<Smite>(e->x, e->y, dmg, r)->source = this;
  Sound::thunder.play(0.25f);
}
