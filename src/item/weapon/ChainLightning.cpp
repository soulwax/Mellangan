#include "item/weapon/ChainLightning.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/Strike.hpp"
#include "level/Level.hpp"

ChainLightning::ChainLightning()
    : Upgrade("Chain Lightning", atlas::kit(atlas::kt::Bluebells),
              {"Lightning leaps across three foes", "Leaps to one more",
               "Leaps to one more", "Faster chains", "+8 damage, one more leap"},
              true) {
  owner = "STORMCALLER";
}

void ChainLightning::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  auto targets = p.level->nearest(
      p.x, p.y, 260, 3 + at(level, 2) + at(level, 3) + at(level, 5));
  if (targets.empty())
    return void(cd = 0.2f);
  cd = (level >= 4 ? 1.4f : 1.8f) * p.haste * pace;
  float dmg = (26.0f + 8 * at(level, 5)) * p.might * power;
  for (Enemy *e : targets)
    p.level->add<Strike>(e->x, e->y, dmg, 16)->source = this;
  Sound::thunder.play(0.3f);
}
