#include "entity/Pickup.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Particle.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"

void Pickup::tick() {
    t += DT;
    z = -std::sin(t * 4 + x) * 1.5;
    Player &p = *level->player;
    if (p.dead()) return;
    double xd = p.x - x, yd = p.bodyY() - y;
    double d = std::sqrt(xd * xd + yd * yd);
    if (!magnet) {
        if (d < reach()) magnet = true, speed = -60;
        return;
    }
    speed += 650 * DT;
    if (d < 5 || d < speed * DT) return take(p), remove();
    x += xd / d * speed * DT;
    y += yd / d * speed * DT;
}

float Pickup::reach() const { return level->player->magnet; }

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

void Gem::render(Screen &s, int xp, int yp) { s.center(atlas::Gems[tier()], xp, yp); }

void Gem::renderLight(Screen &s, int xp, int yp) { s.glow(xp, yp, 26, color() * 0.3f); }

void Gem::take(Player &p) {
    p.gainXp(value);
    Sound::gem.play(0.3f, 0.1f);
    level->add<Spark>(x, y, 0, -20, color(), 0.25f, 0.4f, false);
}

void Potion::render(Screen &s, int xp, int yp) { s.center(atlas::tile(atlas::t::PotionRed), xp, yp, 0.8f); }

void Potion::renderLight(Screen &s, int xp, int yp) { s.glow(xp, yp, 50, {0.35f, 0.06f, 0.08f, 1}); }

float Potion::reach() const { return level->player->magnet * 0.6f; }

void Potion::take(Player &p) {
    p.heal(30);
    Sound::potion.play(0.7f, 0);
    Spark::burst(*level, x, y, col::heal, 10, 50, true);
}

void Magnet::render(Screen &s, int xp, int yp) { s.center(atlas::tile(atlas::t::Coin), xp, yp); }

void Magnet::renderLight(Screen &s, int xp, int yp) { s.glow(xp, yp, 50, {0.10f, 0.25f, 0.45f, 1}); }

void Magnet::take(Player &) {
    level->each<Gem>([](Gem &g) { g.magnet = true, g.speed = 0; });
    Sound::magnet.play(0.7f, 0);
    Spark::burst(*level, x, y, col::xp, 16, 80, true);
}

void Nova::render(Screen &s, int xp, int yp) { s.center(atlas::tile(atlas::t::VialBlue), xp, yp, 0.9f); }

void Nova::renderLight(Screen &s, int xp, int yp) { s.glow(xp, yp, 60, Color{0.3f, 0.3f, 0.45f, 1} * (0.8f + 0.2f * std::sin(t * 6))); }

void Nova::take(Player &p) {
    level->each<Enemy>([&](Enemy &e) {
        double xd = e.x - p.x, yd = e.y - p.y;
        if (xd * xd + yd * yd < 260 * 260) e.hurt(e.type.tile == atlas::t::Cyclops ? 250.0f : 9999.0f, xd, yd, 200);
    });
    Sound::nova.play(0.8f, 0);
    Spark::burst(*level, p.x, p.bodyY(), {0.8f, 0.85f, 1, 1}, 40, 220, true);
    level->game.onExplosion();
}

void Chest::render(Screen &s, int xp, int yp) { s.center(atlas::tile(atlas::t::Chest), xp, yp - 4); }

void Chest::renderLight(Screen &s, int xp, int yp) { s.glow(xp, yp, 110, Color{0.45f, 0.35f, 0.1f, 1} * (1 + 0.2f * std::sin(t * 5))); }

void Chest::take(Player &p) {
    p.pendingLevels++;
    Sound::chest.play(0.8f, 0);
    level->game.announce("TREASURE!");
    Spark::burst(*level, x, y, col::gold, 30, 110, true);
}
