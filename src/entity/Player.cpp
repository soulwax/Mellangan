#include "entity/Player.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Particle.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"

Player::Player() : upgrades(Upgrade::all()) { hp = maxHp = 100; }

Player::~Player() = default;

void Player::onAdded() { upgrades.front()->levelUp(*this); }

void Player::tick() {
    animT += DT;
    flash = 0;
    if (dead()) return;

    Input &in = level->game.input;
    double xd = in.right.down - in.left.down, yd = in.down.down - in.up.down;
    double dd = std::sqrt(xd * xd + yd * yd);
    if (dd > 0) xd /= dd, yd /= dd;
    double k = damp(18, DT);
    xa += (xd * speed - xa) * k;
    ya += (yd * speed - ya) * k;
    x += xa * DT;
    y += ya * DT;
    level->collide(x, y, radius);
    if (xd != 0) facing = xd > 0 ? 1.0f : -1.0f;
    if (xd != 0 || yd != 0) xAim = xd, yAim = yd;
    invuln = std::max(0.0f, invuln - DT);
    hp = std::min(maxHp, hp + regen * DT);

    for (auto &u : upgrades)
        if (u->level > 0) u->tick(*this);
}

void Player::render(Screen &s, int xp, int yp) {
    if (dead()) return;
    bool blink = invuln > 0 && std::fmod(invuln, 0.12f) < 0.06f;
    float move = float(std::sqrt(xa * xa + ya * ya) / speed);
    drawBody(s, xp, yp, atlas::t::Wizard, false, move, col::white, blink ? 0.7f : 0.0f, {1, 0.25f, 0.25f, 1});
    if (hp < maxHp) {
        s.rect(xp - 8, yp + 2, 16, 2, {0.1f, 0.02f, 0.04f, 0.9f});
        s.rect(xp - 8, yp + 2, 16 * hp / maxHp, 2, col::red);
    }
}

void Player::renderShadow(Screen &s, int xp, int yp) {
    if (!dead()) drawShadow(s, xp, yp, false);
}

void Player::renderLight(Screen &s, int xp, int yp) {
    float t = level->time + animT;
    float f = 1 + 0.035f * std::sin(t * 13.1f) + 0.025f * std::sin(t * 7.7f + 1.3f);
    int yb = yp - 7;
    if (dead()) return s.glow(xp, yb, 200, {0.25f, 0.05f, 0.05f, 1});
    s.glow(xp, yb, 300 * f, {0.34f, 0.27f, 0.17f, 1});
    s.glow(xp, yb, 110 * f, {0.22f, 0.16f, 0.08f, 1});
}

void Player::hurt(float damage) {
    if (invuln > 0 || dead() || god) return;
    damage = std::max(1.0f, damage - armor);
    hp -= damage;
    invuln = 0.6f;
    level->game.onPlayerHurt();
    Sound::hurt.play(0.7f);
    TextParticle::pop(*level, x, bodyY(), int(std::lround(damage)), col::red);
    Spark::burst(*level, x, bodyY(), col::red, 6, 60, false);
    if (hp > 0) return;
    hp = 0;
    Spark::burst(*level, x, bodyY(), {0.7f, 0.4f, 1, 1}, 40, 120, true);
    level->game.onPlayerDied();
}

void Player::heal(float amount) {
    hp = std::min(maxHp, hp + amount);
    TextParticle::pop(*level, x, bodyY(), int(amount), col::heal);
}

void Player::gainXp(float amount) {
    for (exp += amount; exp >= expNext; expNext = 6.0f + lvl * 6.0f) exp -= expNext, lvl++, pendingLevels++;
}
