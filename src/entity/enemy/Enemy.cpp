#include "entity/enemy/Enemy.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Cyclops.hpp"
#include "entity/enemy/Ghost.hpp"
#include "entity/enemy/ai/Plans.hpp"
#include "entity/projectile/Orb.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/pickup/Potion.hpp"
#include "level/Level.hpp"
#include "particle/TextParticle.hpp"
#include "particle/debris/Debris.hpp"

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
constexpr int ELITE_XP = 6;
} // namespace

Enemy::Enemy(const EnemyType &type, float hpMul, float damageMul) : type(type), damageMul(damageMul) {
    hp = maxHp = type.hp * hpMul;
    radius = type.radius;
    scale = type.scale;
    animT = rng.range(0, 10);
    seed = rng.next() | 1;
    side = seed & 2 ? 1 : -1;
}

const EnemyType &Enemy::typeOf(EnemyKind kind) { return TYPES[int(kind)]; }

std::unique_ptr<Enemy> Enemy::create(EnemyKind kind, float hpMul, float damageMul) {
    const EnemyType &t = typeOf(kind);
    std::unique_ptr<Enemy> e;
    if (kind == EnemyKind::Ghost) e = std::make_unique<Ghost>(t, hpMul, damageMul);
    else if (kind == EnemyKind::Cyclops) e = std::make_unique<Cyclops>(t, hpMul, damageMul);
    else e = std::make_unique<Enemy>(t, hpMul, damageMul);
    e->plan = planOf(kind);
    return e;
}

float Enemy::roll() {
    seed ^= seed << 13, seed ^= seed >> 17, seed ^= seed << 5;
    return float(seed >> 8) / float(1 << 24);
}

Sense Enemy::sense() const {
    const Player &p = *level->player;
    Sense s{p.x - x, p.y - y, 0, 0, 0, false};
    s.dist = std::sqrt(s.xd * s.xd + s.yd * s.yd);
    if (s.dist > 0.001) s.xn = s.xd / s.dist, s.yn = s.yd / s.dist;
    s.seen = s.dist < 300 && -(p.xAim * s.xn + p.yAim * s.yn) > 0.55;
    return s;
}

void Enemy::next(const Sense &s) {
    if (goals.empty())
        for (int i = 0; i < 6 && plan->steps[i]; i++) goals.push(plan->steps[i]);
    goalTime = 0;
    if (const Goal *g = goals.front()) g->start(*this, s);
}

void Enemy::think() {
    if (!plan) return void(xWant = yWant = 0);
    Sense s = sense();
    for (int i = 0; i < 2 && plan->interrupts[i]; i++) {
        const Goal *g = plan->interrupts[i];
        if (goals.front() != g && g->urgent(*this, s)) goals.pushFront(g), goalTime = 0, g->start(*this, s);
    }
    if (goals.empty()) next(s);
    double xs = 0, ys = 0;
    for (int step = 0; step < 4; step++) {
        const Goal *g = goals.front();
        if (!g || g->tick(*this, s, xs, ys) == GoalState::Running) break;
        goals.pop();
        next(s);
    }
    goalTime += DT, cooldown -= DT;
    xWant = xs, yWant = ys;
}

void Enemy::adopt() {
    wantsLeader = false;
    Enemy *nearby[24];
    for (int i = 0, n = level->gather(x, y, 140, nearby, 24); i < n; i++) {
        Enemy *o = nearby[i];
        if (o != this && &o->type == &type && o->leader == entt::null && o->aware) {
            leader = o->id;
            return;
        }
    }
}

void Enemy::act() {
    if (wantsLeader) adopt();
    if (!wantsFire) return;
    wantsFire = false;
    Player &p = *level->player;
    double xd = p.x - x, yd = p.bodyY() - bodyY(), dd = std::sqrt(xd * xd + yd * yd);
    if (p.dead() || dd < 1e-5) return;
    level->add<Orb>(x, bodyY(), xd / dd * 70, yd / dd * 70, 10 * damageMul);
    Sound::orb.play(0.3f);
}

void Enemy::onAdded() { level->tag(this); }

void Enemy::move() {
    if (!plan) return;
    const Player &p = *level->player;
    x += (xWant + xa) * DT;
    y += (yWant + ya) * DT;
    static const double KNOCK_DECAY = std::exp(-9 * DT);
    xa *= KNOCK_DECAY, ya *= KNOCK_DECAY;
    flash = std::max(0.0f, flash - DT);
    knifeCooldown -= DT;
    animT += DT;
    if (type.floats) z = 4 - std::sin(animT * 7) * 2;
    double xd = p.x - x, yd = p.y - y;
    if (std::abs(xWant) > 4) facing = xWant < 0 ? -1.0f : 1.0f;
    else if (std::abs(xd) > 2) facing = xd < 0 ? -1.0f : 1.0f;
    if (xd * xd + yd * yd > 560 * 560) {
        bool running = p.xa * p.xa + p.ya * p.ya > 100;
        double dir = (running ? std::atan2(p.ya, p.xa) : roll() * TAU) + roll() * 2 - 1, r = 290 + 30 * roll();
        x = p.x + std::cos(dir) * r;
        y = p.y + std::sin(dir) * r;
    }
}

void Enemy::push() {
    xPush = yPush = 0;
    if (!plan || type.floats) return;
    double xd = level->player->x - x, yd = level->player->y - y;
    if (xd * xd + yd * yd > 260 * 260 && ((uintptr_t(this) >> 5) + level->ticks) & 1) return;
    Enemy *nearby[24];
    for (int i = 0, count = level->gather(x, y, radius + 16, nearby, 24); i < count; i++) {
        const Enemy &o = *nearby[i];
        if (&o == this || o.type.floats) continue;
        double ox = x - o.x, oy = y - o.y;
        double min = (radius + o.radius) * 0.85, d2 = ox * ox + oy * oy;
        if (d2 >= min * min || d2 < 1e-6) continue;
        double dd = std::sqrt(d2), share = o.type.speed <= 0 ? 1.0 : o.radius / (radius + o.radius);
        double k = (min - dd) / dd * share;
        xPush += ox * k, yPush += oy * k;
    }
}

void Enemy::settle() {
    if (!plan) return;
    x += xPush, y += yPush;
    if (!type.floats) level->collide(x, y, radius);
}

void Enemy::tick() {
    act();
    Player &p = *level->player;
    double xd = p.x - x, yd = p.y - y, dist = std::sqrt(xd * xd + yd * yd);
    float reach = type.radius + 5;
    if (dist < reach && dist > 1e-3 && !p.dead()) {
        x = p.x - xd / dist * reach;
        y = p.y - yd / dist * reach;
        p.hurt(type.damage * damageMul);
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
    if (const Goal *g = goals.front())
        if (float t = g->tell(*this); t > 0) c = {c.r, c.g * (1 - 0.6f * t), c.b * (1 - 0.6f * t), c.a};
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
    aware = true;
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
