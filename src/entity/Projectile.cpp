#include "entity/Projectile.hpp"

#include <algorithm>

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
