#include "hero/Stormcaller.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Flourish.hpp"
#include "particle/Spark.hpp"
#include "entity/effect/Strike.hpp"

namespace {
bool walker(const Codex &c) { return c.count(c.biomes) >= 10; }
} // namespace

Stormcaller::Stormcaller()
    : Hero({"STORMCALLER", "Storm Rod", "Voice of the thunderheads",
            "Every 20th kill calls down lightning", "walk 10 regions", walker,
            87, {0.5f, 0.78f, 1, 1}, 0, 1, 0, -0.1f, 0, 0, 0, 0.06f,
            "THUNDERCLAP", "Six bolts crash down around you", 8,
            {"Storm Rod", "Lucky Key", "Arcane Staff"}}) {}

namespace {
void strike(Player &p, int count, float damage) {
  Enemy *near[48];
  int n = p.level->gather(p.x, p.y, 240, near, 48);
  for (int i = 0; i < count; i++) {
    double x = p.x + rng.range(-120, 120), y = p.y + rng.range(-90, 90);
    if (n)
      x = near[i % n]->x, y = near[i % n]->y;
    p.level->add<Strike>(x, y, damage * p.might, 24);
  }
  Sound::thunder.play(0.5f);
}
} // namespace

void Stormcaller::cast(Player &p) const {
  strike(p, 6, 50);
  flourish::sigil(*p.level, p.x, p.bodyY(), 90, Color{0.6f, 0.82f, 1, 1}, 6);  p.level->game.onExplosion();
}

void Stormcaller::slay(Player &p, Enemy &) const {
  if (++p.heroCount % 20 == 0)
    strike(p, 3, 40);
}
