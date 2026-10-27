#include "item/weapon/StormRod.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/Strike.hpp"
#include "level/Level.hpp"

StormRod::StormRod()
    : Upgrade("Storm Rod", atlas::tile(atlas::t::StormStaff),
              {"Calls lightning on random foes", "One more strike",
               "+12 damage, wider blast", "One more strike, faster",
               "Two more strikes"},
              true) {}

void StormRod::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  std::vector<Enemy *> pool;
  p.level->each<Enemy>([&](Enemy &e) {
    double xd = e.x - p.x, yd = e.y - p.y;
    if (xd * xd + yd * yd < 230 * 230)
      pool.push_back(&e);
  });
  if (pool.empty())
    return void(cd = 0.2f);
  cd = (level >= 4 ? 1.6f : 2.1f) * p.haste * pace;
  int n = 1 + at(level, 2) + at(level, 4) + 2 * at(level, 5);
  float dmg = (28.0f + 12 * at(level, 3)) * p.might * power,
        r = 14.0f + 8 * at(level, 3);
  for (int i = 0; i < n; i++) {
    Enemy *e = pool[rng.irange(0, int(pool.size()) - 1)];
    p.level->add<Strike>(e->x, e->y, dmg, r)->source = this;
  }
  Sound::thunder.play(0.35f);
}
