#include "entity/Projectile.hpp"

#include <algorithm>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Particle.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"

void Projectile::collide() {
    level->enemiesNear(pos, radius + 32, [&](Enemy &e) {
        if (removed || !touches(e) || std::find(hits.begin(), hits.end(), e.id) != hits.end()) return;
        hits.push_back(e.id);
        e.hurt(damage, push(e), knock);
        if (--pierce <= 0) spent(), remove();
    });
}

bool Projectile::touches(const Enemy &e) const {
    float r = radius + e.radius;
    return lengthSq(e.body() - pos) < r * r;
}

Bolt::Bolt(Vec2 at, Vec2 vel, float damage, int pierce, entt::entity target)
    : Projectile(at, vel, 1.6f, damage, 45, pierce), target(target) {}

void Bolt::tick() {
    if ((life -= DT) <= 0) return remove();
    if (Enemy *e = level->get<Enemy>(target)) vel = lerp(vel, normalized(e->body() - pos) * 230, damp(6, DT));
    pos += vel * DT;
    if (rng.chance(0.5f)) level->add<Spark>(pos, vel * -0.1f + rng.onCircle() * 8, Color{0.4f, 0.75f, 1, 1}, 0.25f, 0.35f, false);
    collide();
}

void Bolt::render(Screen &s) {
    s.color({0.45f, 0.8f, 1, 1});
    s.center(atlas::Bolt, pos, 1.1f);
    s.color(col::white);
    s.center(atlas::Dot, pos, 0.45f);
}

void Bolt::renderLight(Screen &s) { s.glow(pos, 52, {0.06f, 0.20f, 0.36f, 1}); }

void Bolt::spent() { Spark::burst(*level, pos, {0.45f, 0.8f, 1, 1}, 6, 60, true); }

Slash::Slash(Player &p, float side, float damage, float scale)
    : Projectile(p.body(), {}, 0.22f, damage, 90, 9999), player(p), side(side), scale(scale) {}

void Slash::tick() {
    if ((life -= DT) <= 0) return remove();
    pos = player.body() + Vec2{side * 16 * scale, 0};
    collide();
}

bool Slash::touches(const Enemy &e) const {
    Vec2 d = e.body() - pos;
    float rx = 14 * scale + e.radius * 0.5f, ry = 24 * scale;
    return (d.x * d.x) / (rx * rx) + (d.y * d.y) / (ry * ry) <= 1;
}

void Slash::render(Screen &s) {
    float f = frac();
    s.color({1, 0.95f, 0.82f, std::pow(f, 0.6f)});
    s.sprite(atlas::Slash, pos, {12, 24}, {side * scale * (0.8f + 0.4f * (1 - f)), scale});
    s.color(col::white);
}

void Slash::renderLight(Screen &s) { s.glow(pos, 80 * scale, Color{0.16f, 0.15f, 0.11f, 1} * frac()); }

Axe::Axe(Vec2 at, Vec2 vel, float damage, int pierce, float spin)
    : Projectile(at, vel, 2.2f, damage, 60, pierce), spin(spin) { radius = 8; }

void Axe::tick() {
    if ((life -= DT) <= 0) return remove();
    vel.y += 560 * DT;
    pos += vel * DT;
    rot += spin * DT;
    collide();
}

void Axe::render(Screen &s) { s.center(atlas::tile(atlas::t::Axe), pos, 1, rot); }

void Axe::renderLight(Screen &s) { s.glow(pos, 44, {0.20f, 0.10f, 0.03f, 1}); }

void Arrow::tick() {
    if ((life -= DT) <= 0) return remove();
    pos += vel * DT;
    collide();
}

void Arrow::render(Screen &s) { s.center(atlas::town(atlas::tt::Arrow), pos, 1, std::atan2(vel.y, vel.x) + PI / 4); }

Strike::Strike(Vec2 at, float damage, float radius) : damage(damage) {
    pos = at, this->radius = radius;
    for (float &j : jag) j = rng.range(-7, 7);
}

void Strike::tick() {
    if (!struck) {
        struck = true;
        level->enemiesNear(pos, radius + 24, [&](Enemy &e) {
            float r = radius + e.radius;
            if (lengthSq(e.body() - pos) < r * r) e.hurt(damage, normalized(e.pos - pos), 50);
        });
        Spark::burst(*level, pos, {0.7f, 0.85f, 1, 1}, 10, 90, true);
    }
    if ((life -= DT) <= 0) remove();
}

void Strike::render(Screen &s) {
    float a = life / 0.3f;
    Vec2 prev = pos - Vec2{0, 150};
    for (size_t i = 0; i < jag.size(); i++) {
        float k = float(i + 1) / float(jag.size());
        Vec2 next = i + 1 == jag.size() ? pos : pos + Vec2{jag[i], -150 * (1 - k)};
        s.line(prev, next, 3, {0.55f, 0.75f, 1, a * 0.7f});
        s.line(prev, next, 1, col::white.alpha(a));
        prev = next;
    }
    s.color({0.7f, 0.85f, 1, a * 0.6f});
    s.center(atlas::Ring, pos, radius * 2 / atlas::Ring.w * (1.3f - 0.3f * a));
    s.color(col::white);
}

void Strike::renderLight(Screen &s) { s.glow(pos, 160, Color{0.35f, 0.45f, 0.7f, 1} * (life / 0.3f)); }

void FirePool::tick() {
    if ((life -= DT) <= 0) return remove();
    if ((timer -= DT) <= 0) {
        timer = 0.4f;
        level->enemiesNear(pos, radius + 24, [&](Enemy &e) {
            float r = radius + e.radius;
            if (lengthSq(e.pos - pos) < r * r) e.hurt(damage, {}, 0);
        });
    }
    if (rng.chance(0.6f) && level->count<Spark>() < 500) {
        Vec2 at = pos + rng.onCircle() * rng.range(0, radius);
        level->add<Spark>(at, Vec2{rng.range(-6, 6), -rng.range(15, 35)}, Color{1, rng.range(0.35f, 0.7f), 0.15f, 1},
                          rng.range(0.3f, 0.6f), rng.range(0.3f, 0.6f), false);
    }
}

void FirePool::render(Screen &s) {
    float fade = std::min(1.0f, life * 2) * std::min(1.0f, (maxLife - life) * 6);
    s.color({1, 0.45f, 0.12f, 0.35f * fade});
    s.center(atlas::Glow, pos, radius * 2.4f / atlas::Glow.w);
    s.color({1, 0.6f, 0.2f, 0.25f * fade});
    s.center(atlas::Ring, pos, radius * 2 / atlas::Ring.w, -life);
    s.color(col::white);
}

void FirePool::renderLight(Screen &s) {
    float flicker = 0.85f + 0.15f * std::sin(life * 23) * std::sin(life * 7);
    s.glow(pos, radius * 4, Color{0.45f, 0.22f, 0.05f, 1} * (flicker * std::min(1.0f, life * 2)));
}

void Bomb::tick() {
    t += DT / 0.55f;
    pos = lerp(from, to, std::min(t, 1.0f));
    height = std::sin(std::min(t, 1.0f) * PI) * 40;
    if (t < 1) return;
    level->enemiesNear(pos, 28 + 24, [&](Enemy &e) {
        float r = 28 + e.radius;
        if (lengthSq(e.pos - pos) < r * r) e.hurt(damage, normalized(e.pos - pos), 90);
    });
    Spark::burst(*level, pos, {1, 0.55f, 0.2f, 1}, 16, 110, true);
    level->add<FirePool>(pos, poolRadius, poolDamage, poolTime);
    Sound::explosion.play(0.5f);
    level->game.onExplosion();
    remove();
}

void Bomb::render(Screen &s) {
    s.sprite(atlas::Shadow, pos + Vec2{0, 1}, {8, 4}, {0.6f, 0.6f});
    s.center(atlas::town(atlas::tt::Bomb), pos - Vec2{0, height + 6}, 1, t * 9);
}

Sweep::Sweep(const Player &p, float reach, float damage)
    : player(p), reach(reach), damage(damage), angle(rng.range(0, TAU)), start(angle) { pos = p.body(); }

void Sweep::tick() {
    t += DT / 0.45f;
    angle = start + TAU * std::min(t, 1.0f);
    pos = player.body() + fromAngle(angle) * reach;
    level->enemiesNear(pos, 40, [&](Enemy &e) {
        float r = 13 + e.radius;
        if (lengthSq(e.body() - pos) >= r * r || std::find(hits.begin(), hits.end(), e.id) != hits.end()) return;
        hits.push_back(e.id);
        e.hurt(damage, normalized(e.pos - player.pos), 120);
    });
    if (t >= 1) remove();
}

void Sweep::render(Screen &s) {
    for (int i = 3; i >= 1; i--) {
        Vec2 ghost = player.body() + fromAngle(angle - i * 0.28f) * reach;
        s.color({0.8f, 0.85f, 1, 0.15f * float(4 - i)});
        s.center(atlas::town(atlas::tt::Scythe), ghost, 1.2f, angle - i * 0.28f + PI * 0.75f);
    }
    s.color(col::white);
    s.center(atlas::town(atlas::tt::Scythe), pos, 1.2f, angle + PI * 0.75f);
}

void Sweep::renderLight(Screen &s) { s.glow(pos, 60, {0.18f, 0.2f, 0.28f, 1}); }

void Orb::tick() {
    if ((life -= DT) <= 0) return remove();
    pos += vel * DT;
    Player &p = *level->player;
    if (!p.dead() && lengthSq(p.body() - pos) < 7 * 7) {
        p.hurt(damage);
        Spark::burst(*level, pos, {0.75f, 0.35f, 1, 1}, 6, 50, false);
        remove();
    }
}

void Orb::render(Screen &s) {
    s.color({0.75f, 0.35f, 1, 1});
    s.center(atlas::Bolt, pos, 1.2f, life * 4);
    s.color(col::white);
    s.center(atlas::Dot, pos, 0.4f);
}

void Orb::renderLight(Screen &s) { s.glow(pos, 44, {0.25f, 0.08f, 0.35f, 1}); }
