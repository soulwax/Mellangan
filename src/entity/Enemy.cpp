#include "entity/Enemy.hpp"

#include <spdlog/spdlog.h>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Particle.hpp"
#include "entity/Pickup.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"

namespace {
namespace t = atlas::t;
// tile, hp, speed, damage, radius, scale, xp, blood, floats, unlockAt
const EnemyType TYPES[] = {
    {t::Bat, 5, 50, 5, 5, 1, 1, {0.85f, 0.55f, 0.35f, 1}, true, 0},
    {t::Rat, 9, 40, 6, 6, 1, 1, {0.75f, 0.55f, 0.4f, 1}, false, 25},
    {t::Slime, 16, 30, 8, 6, 1, 2, {0.3f, 0.95f, 0.6f, 1}, false, 50},
    {t::Spider, 22, 48, 10, 6, 1, 3, {0.9f, 0.6f, 0.4f, 1}, false, 100},
    {t::Ghost, 30, 36, 12, 6, 1, 4, {0.8f, 0.85f, 1, 1}, true, 160},
    {t::Demon, 70, 32, 16, 8, 1.25f, 8, {1, 0.3f, 0.25f, 1}, false, 240},
    {t::Cyclops, 900, 30, 28, 15, 2.5f, 40, {1, 0.75f, 0.5f, 1}, false, 1e9f},
};
} // namespace

Enemy::Enemy(const EnemyType &type, float hpMul, float damageMul) : type(type), damageMul(damageMul) {
    hp = maxHp = type.hp * hpMul;
    radius = type.radius;
    scale = type.scale;
    animT = rng.range(0, 10);
}

const EnemyType &Enemy::typeOf(EnemyKind kind) { return TYPES[int(kind)]; }

std::unique_ptr<Enemy> Enemy::create(EnemyKind kind, float hpMul, float damageMul) {
    const EnemyType &t = typeOf(kind);
    switch (kind) {
    case EnemyKind::Bat: return std::make_unique<Bat>(t, hpMul, damageMul);
    case EnemyKind::Ghost: return std::make_unique<Ghost>(t, hpMul, damageMul);
    case EnemyKind::Cyclops: return std::make_unique<Cyclops>(t, hpMul, damageMul);
    default: return std::make_unique<Enemy>(t, hpMul, damageMul);
    }
}

void Enemy::onAdded() { level->tag(this); }

void Enemy::tick() {
    Player &p = *level->player;
    Vec2 to = p.pos - pos;
    float dist = length(to);
    Vec2 dir = dist > 0.001f ? to / dist : Vec2{};

    pos += (steer(dir) + knock) * DT;
    knock *= std::exp(-9 * DT);
    flash = std::max(0.0f, flash - DT);
    knifeCooldown -= DT;
    animT += DT;
    if (std::abs(dir.x) > 0.15f) facing = dir.x < 0 ? -1.0f : 1.0f;

    if (dist > 560) {
        Vec2 ahead = lengthSq(p.vel) > 100 ? normalized(p.vel) : rng.onCircle();
        pos = p.pos + rotate(ahead, rng.range(-1, 1)) * rng.range(290, 320);
        return;
    }
    float reach = type.radius + 5;
    if (dist < reach && !p.dead()) {
        pos = p.pos - dir * reach;
        p.hurt(type.damage * damageMul);
    }
    if (!type.floats) separate();
}

void Enemy::separate() {
    level->enemiesNear(pos, radius + 16, [&](Enemy &o) {
        if (&o == this || o.type.floats) return;
        Vec2 d = pos - o.pos;
        float min = (radius + o.radius) * 0.85f, d2 = lengthSq(d);
        if (d2 >= min * min || d2 < 1e-6f) return;
        float dd = std::sqrt(d2), w = o.radius / (radius + o.radius);
        Vec2 n = d / dd * ((min - dd) * 0.5f);
        pos += n * w;
        o.pos -= n * (1 - w);
    });
}

void Enemy::render(Screen &s) { drawBody(s, type.tile, type.floats, 1, tint(), flash > 0 ? 1.0f : 0.0f, col::white); }

void Enemy::renderShadow(Screen &s) { drawShadow(s, type.floats); }

void Enemy::hurt(float damage, Vec2 dir, float force) {
    if (removed) return;
    bool crit = rng.chance(0.08f);
    if (crit) damage *= 1.8f;
    hp -= damage;
    flash = 0.1f;
    knock += dir * (force * knockResist);
    TextParticle::pop(*level, body(), int(std::lround(damage)), crit ? col::gold : col::white);
    Sound::hit.play(0.3f);
    if (hp <= 0) die();
}

void Enemy::die() {
    remove();
    level->kills++;
    Sound::kill.play(0.3f, 0.12f);
    Gem::drop(*level, pos + Vec2{0, -3}, type.xp);
    if (rng.chance(1 / 150.0f)) level->add<Potion>(pos);
    Spark::burst(*level, body(), type.blood, 8, 70, false);
}

Vec2 Bat::steer(Vec2 dir) { return dir * type.speed + perp(dir) * (std::sin(animT * 5) * 34); }

Vec2 Ghost::steer(Vec2 dir) { return dir * (type.speed * (0.8f + 0.4f * std::sin(animT * 2))); }

void Ghost::renderLight(Screen &s) { s.glow(body(), 40, {0.06f, 0.10f, 0.18f, 1}); }

Cyclops::Cyclops(const EnemyType &type, float hpMul, float damageMul) : Enemy(type, hpMul, damageMul) { knockResist = 0.15f; }

void Cyclops::renderLight(Screen &s) { s.glow(body(), 130, {0.35f, 0.08f, 0.04f, 1}); }

void Cyclops::die() {
    Enemy::die();
    level->add<Chest>(pos);
    Spark::burst(*level, body(), type.blood, 40, 140, true);
    level->game.announce("THE CYCLOPS FALLS");
}
