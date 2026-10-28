#include "hero/Alchemist.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"
#include "entity/pickup/Potion.hpp"
#include "entity/projectile/Bomb.hpp"

namespace {
bool hoarder(const Codex &c) { return c.count(c.relics) >= 3; }
} // namespace

Alchemist::Alchemist()
    : Hero({"ALCHEMIST", "Fire Bomb", "Brewer of fire and cures",
            "Fallen foes may leave a potion", "find 3 relics", hoarder, 100,
            {1, 0.6f, 0.3f, 1}, 0, 1, 0, 0, 14, 0, 0.5f, 0, "ELIXIR",
            "Drink to heal 25, hurl three bombs", 12,
            {"Fire Bomb", "Troll Blood", "Lodestone"}}) {}

void Alchemist::cast(Player &p) const {
  p.heal(25);
  for (int i = 0; i < 3; i++) {
    double dir = TAU * i / 3 + rng.range(0, 1), r = rng.range(50, 90);
    p.level->add<Bomb>(p.x, p.bodyY(), p.x + std::cos(dir) * r,
                       p.y + std::sin(dir) * r, 20 * p.might, 28,
                       7 * p.might, 3);
  }
  Sound::axe.play(0.6f, 0);
}

void Alchemist::slay(Player &p, Enemy &e) const {
  if (rng.chance(0.03f))
    p.level->add<Potion>(e.x, e.y);
}
