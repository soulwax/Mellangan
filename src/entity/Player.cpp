#include "entity/Player.hpp"

#include "Game.hpp"
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
    Vec2 dir = normalized({float(in.right.down - in.left.down), float(in.down.down - in.up.down)});
    vel = lerp(vel, dir * speed, damp(18, DT));
    pos += vel * DT;
    if (dir.x != 0) facing = dir.x > 0 ? 1.0f : -1.0f;
    invuln = std::max(0.0f, invuln - DT);

    for (auto &u : upgrades)
        if (u->level > 0) u->tick(*this);
}

void Player::render(Screen &s) {
    if (dead()) return;
    bool blink = invuln > 0 && std::fmod(invuln, 0.12f) < 0.06f;
    drawBody(s, atlas::t::Wizard, false, length(vel) / speed, col::white, blink ? 0.7f : 0.0f, {1, 0.25f, 0.25f, 1});
    if (hp < maxHp) {
        s.rect(pos.x - 8, pos.y + 2, 16, 2, {0.1f, 0.02f, 0.04f, 0.9f});
        s.rect(pos.x - 8, pos.y + 2, 16 * hp / maxHp, 2, col::red);
    }
}

void Player::renderShadow(Screen &s) {
    if (!dead()) drawShadow(s, false);
}

void Player::renderLight(Screen &s) {
    float t = level->time + animT;
    float f = 1 + 0.035f * std::sin(t * 13.1f) + 0.025f * std::sin(t * 7.7f + 1.3f);
    if (dead()) return s.glow(body(), 200, {0.25f, 0.05f, 0.05f, 1});
    s.glow(body(), 430 * f, {0.30f, 0.25f, 0.18f, 1});
    s.glow(body(), 130 * f, {0.20f, 0.15f, 0.09f, 1});
}

void Player::hurt(float damage) {
    if (invuln > 0 || dead()) return;
    hp -= damage;
    invuln = 0.6f;
    level->game.onPlayerHurt();
    TextParticle::pop(*level, body(), int(std::lround(damage)), col::red);
    Spark::burst(*level, body(), col::red, 6, 60, false);
    if (hp > 0) return;
    hp = 0;
    Spark::burst(*level, body(), {0.7f, 0.4f, 1, 1}, 40, 120, true);
    level->game.onPlayerDied();
}

void Player::heal(float amount) {
    hp = std::min(maxHp, hp + amount);
    TextParticle::pop(*level, body(), int(amount), col::heal);
}

void Player::gainXp(float amount) {
    for (xp += amount; xp >= xpNext; xpNext = 6.0f + lvl * 6.0f) xp -= xpNext, lvl++, pendingLevels++;
}
