#include "entity/Pickup.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Particle.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"

void Pickup::tick() {
    t += DT;
    Player &p = *level->player;
    if (p.dead()) return;
    Vec2 to = p.body() - pos;
    float d = length(to);
    if (!magnet) {
        if (d < reach()) magnet = true, speed = -60;
        return;
    }
    speed += 650 * DT;
    if (d < 5 || d < speed * DT) return take(p), remove();
    pos += to / d * speed * DT;
}

float Pickup::reach() const { return level->player->magnet; }

void Gem::drop(Level &level, Vec2 at, float value) {
    if (level.count<Gem>() > 450) {
        if (Gem *g = level.any<Gem>()) return void(g->value += value);
    }
    level.add<Gem>(at, value);
}

void Gem::onAdded() { level->tag(this); }

void Gem::tick() {
    pos += vel * DT;
    vel *= std::exp(-5 * DT);
    Pickup::tick();
}

Color Gem::color() const {
    constexpr Color c[3] = {{0.35f, 0.65f, 1, 1}, {0.35f, 1, 0.55f, 1}, {1, 0.35f, 0.45f, 1}};
    return c[tier()];
}

void Gem::render(Screen &s) { s.center(atlas::Gems[tier()], pos + Vec2{0, bob()}); }

void Gem::renderLight(Screen &s) { s.glow(pos, 26, color() * 0.3f); }

void Gem::take(Player &p) {
    p.gainXp(value);
    Sound::gem.play(0.3f, 0.1f);
    level->add<Spark>(pos, Vec2{0, -20}, color(), 0.25f, 0.4f, false);
}

void Potion::render(Screen &s) { s.center(atlas::tile(atlas::t::PotionRed), pos + Vec2{0, bob()}, 0.8f); }

void Potion::renderLight(Screen &s) { s.glow(pos, 50, {0.35f, 0.06f, 0.08f, 1}); }

float Potion::reach() const { return level->player->magnet * 0.6f; }

void Potion::take(Player &p) {
    p.heal(30);
    Sound::potion.play(0.7f, 0);
    Spark::burst(*level, pos, col::heal, 10, 50, true);
}

void Chest::render(Screen &s) { s.center(atlas::tile(atlas::t::Chest), pos + Vec2{0, bob() - 4}); }

void Chest::renderLight(Screen &s) { s.glow(pos, 110, Color{0.45f, 0.35f, 0.1f, 1} * (1 + 0.2f * std::sin(t * 5))); }

void Chest::take(Player &p) {
    p.pendingLevels++;
    Sound::chest.play(0.8f, 0);
    level->game.announce("TREASURE!");
    Spark::burst(*level, pos, col::gold, 30, 110, true);
}
