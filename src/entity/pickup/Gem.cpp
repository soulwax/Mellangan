#include "entity/pickup/Gem.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/Pickup.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

void Gem::drop(Level &level, double x, double y, float value) {
    if (level.count<Gem>() > 450) {
        if (Gem *g = level.any<Gem>()) return void(g->value += value);
    }
    level.add<Gem>(x, y, value);
}

void Gem::onAdded() { level->tag(this); }

void Gem::tick() {
    x += xa * DT;
    y += ya * DT;
    double k = std::exp(-5 * DT);
    xa *= k, ya *= k;
    Pickup::tick();
}

Color Gem::color() const {
    constexpr Color c[3] = {{0.35f, 0.65f, 1, 1}, {0.35f, 1, 0.55f, 1}, {1, 0.35f, 0.45f, 1}};
    return c[tier()];
}

void Gem::render(Screen &s) { s.center(atlas::Gems[tier()], xp, yp); }

void Gem::renderLight(Screen &s) { s.glow(xp, yp, 26, color() * 0.3f); }

void Gem::take(Player &p) {
    p.gainXp(value);
    Sound::gem.play(0.3f, 0.1f);
    level->add<Spark>(x, y, 0, -20, color(), 0.25f, 0.4f, false);
}
