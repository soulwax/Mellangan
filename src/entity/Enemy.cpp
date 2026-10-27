#include "entity/Enemy.hpp"

#include <spdlog/spdlog.h>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Pickup.hpp"
#include "entity/Player.hpp"
#include "entity/Projectile.hpp"
#include "level/Level.hpp"
#include "particle/Debris.hpp"
#include "particle/Particle.hpp"

namespace {
namespace t = atlas::t;

const EnemyType TYPES[] = {
    {t::Bat, 5, 50, 5, 5, 1, 1, 1, {0.85f, 0.55f, 0.35f, 1}, true, 0},
    {t::Rat, 9, 40, 6, 6, 1, 1, 1, {0.75f, 0.55f, 0.4f, 1}, false, 25},
    {t::Slime, 16, 30, 8, 6, 1, 2, 1, {0.3f, 0.95f, 0.6f, 1}, false, 50},
    {t::Scorpion, 14, 58, 7, 6, 1, 2, 1, {0.7f, 0.75f, 0.8f, 1}, false, 70},
    {t::Spider, 22, 48, 10, 6, 1, 3, 1, {0.9f, 0.6f, 0.4f, 1}, false, 100},
    {t::Ghost, 30, 36, 12, 6, 1, 4, 1, {0.8f, 0.85f, 1, 1}, true, 160},
    {t::Cultist, 40, 38, 12, 6, 1, 5, 1, {0.6f, 0.3f, 0.8f, 1}, false, 200},
    {t::Demon, 70, 32, 16, 8, 1.25f, 8, 0.8f, {1, 0.3f, 0.25f, 1}, false, 240},
    {t::Knight, 120, 26, 18, 7, 1.15f, 10, 0.4f, {0.75f, 0.78f, 0.85f, 1}, false, 300},
    {t::Cyclops, 900, 30, 28, 15, 2.5f, 40, 0.15f, {1, 0.75f, 0.5f, 1}, false, 1e9f},
};
const EnemyType BARREL{t::Barrel, 12, 0, 0, 6, 1, 0, 0, {0.6f, 0.4f, 0.25f, 1}, false, 1e9f};
constexpr int ELITE_XP = 6;
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
    case EnemyKind::Scorpion: return std::make_unique<Scorpion>(t, hpMul, damageMul);
    case EnemyKind::Cultist: return std::make_unique<Cultist>(t, hpMul, damageMul);
    case EnemyKind::Ghost: return std::make_unique<Ghost>(t, hpMul, damageMul);
    case EnemyKind::Cyclops: return std::make_unique<Cyclops>(t, hpMul, damageMul);
    default: return std::make_unique<Enemy>(t, hpMul, damageMul);
    }
}

void Enemy::onAdded() { level->tag(this); }

void Enemy::tick() {
    Player &p = *level->player;
    double xd = p.x - x, yd = p.y - y;
    double dist = std::sqrt(xd * xd + yd * yd);
    double xn = 0, yn = 0;
    if (dist > 0.001) xn = xd / dist, yn = yd / dist;

    double xs, ys;
    steer(xn, yn, xs, ys);
    x += (xs + xa) * DT;
    y += (ys + ya) * DT;
    static const double KNOCK_DECAY = std::exp(-9 * DT);
    xa *= KNOCK_DECAY, ya *= KNOCK_DECAY;
    flash = std::max(0.0f, flash - DT);
    knifeCooldown -= DT;
    animT += DT;
    if (type.floats) z = 4 - std::sin(animT * 7) * 2;
    if (std::abs(xn) > 0.15) facing = xn < 0 ? -1.0f : 1.0f;

    if (dist > 560) {
        double dir = p.xa * p.xa + p.ya * p.ya > 100 ? std::atan2(p.ya, p.xa) : rng.range(0, TAU);
        dir += rng.range(-1, 1);
        double r = rng.range(290, 320);
        x = p.x + std::cos(dir) * r;
        y = p.y + std::sin(dir) * r;
        return;
    }
    float reach = type.radius + 5;
    if (dist < reach && !p.dead()) {
        x = p.x - xn * reach;
        y = p.y - yn * reach;
        p.hurt(type.damage * damageMul);
    }
    if (type.floats) return;
    if (dist < 260 || ((uintptr_t(this) >> 5) + level->ticks) & 1) separate();
    level->collide(x, y, radius);
}

void Enemy::separate() {
    Enemy *nearby[24];
    for (int i = 0, count = level->gather(x, y, radius + 16, nearby, 24); i < count; i++) {
        Enemy &o = *nearby[i];
        if (&o == this || o.type.floats || (&o < this && o.type.speed > 0)) continue;
        double xd = x - o.x, yd = y - o.y;
        double min = (radius + o.radius) * 0.85, d2 = xd * xd + yd * yd;
        if (d2 >= min * min || d2 < 1e-6) continue;
        double dd = std::sqrt(d2), w = o.type.speed <= 0 ? 1.0 : o.radius / (radius + o.radius);
        double push = (min - dd) / dd;
        x += xd * push * w;
        y += yd * push * w;
        o.x -= xd * push * (1 - w);
        o.y -= yd * push * (1 - w);
    }
}

void Enemy::makeElite() {
    elite = true;
    hp = maxHp *= 4;
    scale *= 1.35f;
    radius *= 1.3f;
}

void Enemy::render(Screen &s) {
    Color c = tint();
    if (elite) c = {c.r, c.g * 0.85f, c.b * 0.45f, c.a};
    drawBody(s, xp, yp, type.tile, type.floats, 1, c, flash > 0 ? 1.0f : 0.0f, col::white);
}

void Enemy::renderLight(Screen &s) {
    if (elite) s.glow(xp, yp - 7 * scale, 40 * scale, {0.12f, 0.09f, 0.02f, 1});
}

void Enemy::renderShadow(Screen &s) { drawShadow(s, xp, yp, type.floats); }

void Enemy::hurt(float damage, double xd, double yd, float force) {
    if (removed) return;
    double dd = std::sqrt(xd * xd + yd * yd);
    if (dd > 1e-5) xd /= dd, yd /= dd;
    else xd = yd = 0;
    bool crit = rng.chance(0.08f + level->player->luck);
    if (crit) damage *= 1.8f;
    hp -= damage;
    flash = 0.1f;
    float f = force * type.knockResist * (elite ? 0.5f : 1.0f);
    xa += xd * f;
    ya += yd * f;
    TextParticle::pop(*level, x, bodyY(), int(std::lround(damage)), crit ? col::gold : col::white);
    Sound::hit.play(0.35f);
    if (hp <= 0) die();
}

void Enemy::die() {
    remove();
    level->kills++;
    Sound::kill.play(0.35f, 0.12f);
    Gem::drop(*level, x, y - 3, type.xp * (elite ? ELITE_XP : 1));
    if (elite && rng.chance(0.25f)) level->add<Potion>(x + 6, y);
    if (rng.chance(1 / 150.0f)) level->add<Potion>(x, y);
    Debris::gore(*level, x, y, 7 * scale, type.blood, !type.floats);
}

void Bat::steer(double xn, double yn, double &xs, double &ys) {
    double w = std::sin(animT * 5) * 34;
    xs = xn * type.speed - yn * w;
    ys = yn * type.speed + xn * w;
}

void Scorpion::steer(double xn, double yn, double &xs, double &ys) {
    double v = type.speed * (0.5f + std::max(0.0f, std::sin(animT * 4)) * 1.2f);
    xs = xn * v;
    ys = yn * v;
}

void Cultist::steer(double xn, double yn, double &xs, double &ys) {
    double w = std::cos(animT * 3) * 28;
    xs = xn * type.speed - yn * w;
    ys = yn * type.speed + xn * w;
}

void Cultist::tick() {
    Enemy::tick();
    if (removed || (castTimer -= DT) > 0) return;
    castTimer = rng.range(2.5f, 4.0f);
    Player &p = *level->player;
    double xd = p.x - x, yd = p.bodyY() - bodyY();
    double dd = std::sqrt(xd * xd + yd * yd);
    if (p.dead() || dd > 200 || dd < 1e-5) return;
    level->add<Orb>(x, bodyY(), xd / dd * 70, yd / dd * 70, 10 * damageMul);
    Sound::orb.play(0.3f);
}

SupplyBarrel::SupplyBarrel() : Enemy(BARREL, 1, 0) {}

void SupplyBarrel::tick() {
    flash = std::max(0.0f, flash - DT);
    animT += DT;
    double xd = level->player->x - x, yd = level->player->y - y;
    if (xd * xd + yd * yd > 700 * 700) remove();
}

void SupplyBarrel::render(Screen &s) {
    s.flash(atlas::tile(t::Barrel), xp, yp, 8, 16, 1, 1, 0, flash > 0 ? 1.0f : 0.0f, col::white);
}

void SupplyBarrel::renderLight(Screen &s) {
    s.glow(xp, yp - 7 * scale, 50, Color{0.22f, 0.16f, 0.06f, 1} * (0.8f + 0.2f * std::sin(animT * 3)));
}

void SupplyBarrel::die() {
    remove();
    Sound::barrel.play(0.6f);
    Spark::burst(*level, x, bodyY(), type.blood, 12, 80, false);
    float r = rng.unit();
    if (r < 0.45f) level->add<Potion>(x, y);
    else if (r < 0.8f) level->add<Magnet>(x, y);
    else level->add<Nova>(x, y);
}

void Ghost::steer(double xn, double yn, double &xs, double &ys) {
    double v = type.speed * (0.8f + 0.4f * std::sin(animT * 2));
    xs = xn * v;
    ys = yn * v;
}

void Ghost::renderLight(Screen &s) {
    Enemy::renderLight(s);
    s.glow(xp, yp - 7 * scale, 40, {0.06f, 0.10f, 0.18f, 1});
}

void Cyclops::renderLight(Screen &s) { s.glow(xp, yp - 7 * scale, 130, {0.35f, 0.08f, 0.04f, 1}); }

void Cyclops::die() {
    Enemy::die();
    level->add<Chest>(x, y);
    Spark::burst(*level, x, bodyY(), type.blood, 40, 140, true);
    level->add<Explosion>(x, y, 10, 9);
    level->game.announce("THE CYCLOPS FALLS");
}
