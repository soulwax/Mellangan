#include "item/relic/TempestEye.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/Strike.hpp"
#include "level/Level.hpp"

TempestEye::TempestEye()
    : Upgrade("Tempest Eye", atlas::kit(atlas::kt::Amethyst),
              {"Three bolts of lightning, every second", "", "", "", ""},
              true) {
  rare = true;
}

void TempestEye::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  Enemy *pool[64];
  int n = p.level->gather(p.x, p.y, 260, pool, 64);
  if (n == 0)
    return void(cd = 0.2f);
  cd = 0.9f * p.haste;
  for (int i = 0; i < 3; i++) {
    Enemy *e = pool[rng.irange(0, n - 1)];
    p.level->add<Strike>(e->x, e->y, 55 * p.might, 26)->source = this;
  }
  Sound::thunder.play(0.4f);
}
