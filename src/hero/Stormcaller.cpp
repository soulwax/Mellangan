#include "hero/Stormcaller.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/Strike.hpp"
#include "level/Level.hpp"

namespace {
bool walker(const Codex &c) { return c.count(c.biomes) >= 10; }
} // namespace

Stormcaller::Stormcaller()
    : Hero({"STORMCALLER", "Storm Rod", "Voice of the thunderheads",
            "Every 20th kill calls down lightning", "walk 10 regions", walker,
            87, 0, 1, 0, -0.1f, 0, 0, 0, 0.06f}) {}

void Stormcaller::slay(Player &p, Enemy &) const {
  if (++p.heroCount % 20)
    return;
  Enemy *near[32];
  int n = p.level->gather(p.x, p.y, 220, near, 32);
  for (int i = 0; i < std::min(n, 3); i++)
    p.level->add<Strike>(near[i]->x, near[i]->y, 40 * p.might, 24);
  Sound::thunder.play(0.4f);
}
