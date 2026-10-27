#include "entity/pickup/Chest.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

void Chest::render(Screen &s) {
  s.center(atlas::tile(atlas::t::Chest), xp, yp - 4);
}

void Chest::renderLight(Screen &s) {
  s.glow(xp, yp, 110,
         Color{0.45f, 0.35f, 0.1f, 1} * (1 + 0.2f * std::sin(t * 5)));
}

void Chest::take(Player &p) {
  p.pendingLevels++, p.pendingTreasures++;
  Sound::chest.play(0.8f, 0);
  level->game.announce("TREASURE!");
  Spark::burst(*level, x, y, col::gold, 30, 110, true);
}
