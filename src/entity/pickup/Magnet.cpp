#include "entity/pickup/Magnet.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/Gem.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

void Magnet::render(Screen &s) { s.center(atlas::tile(atlas::t::Coin), xp, yp); }

void Magnet::renderLight(Screen &s) { s.glow(xp, yp, 50, {0.10f, 0.25f, 0.45f, 1}); }

void Magnet::take(Player &) {
    level->each<Gem>([](Gem &g) { g.magnet = true, g.speed = 0; });
    Sound::magnet.play(0.7f, 0);
    Spark::burst(*level, x, y, col::xp, 16, 80, true);
}
