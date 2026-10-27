#include "entity/Projectile.hpp"

#include <algorithm>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Debris.hpp"
#include "particle/Particle.hpp"

void Projectile::collide() {
    level->enemiesNear(x, y, radius + 32, [&](Enemy &e) {
        if (removed || !touches(e) || std::find(hits.begin(), hits.end(), e.id) != hits.end()) return;
        hits.push_back(e.id);
        double xd, yd;
        push(e, xd, yd);
        e.hurt(damage, xd, yd, knock);
        if (--pierce <= 0) spent(), remove();
    });
}

bool Projectile::touches(const Enemy &e) const {
    double xd = e.x - x, yd = e.bodyY() - y;
    float r = radius + e.radius;
    return xd * xd + yd * yd < r * r;
}

Bolt::Bolt(double x, double y, double xa, double ya, float damage, int pierce, entt::entity target)
    : Projectile(x, y, xa, ya, 1.6f, damage, 45, pierce), target(target) {}

void Bolt::tick() {
    if ((life -= DT) <= 0) return remove();
    if (Enemy *e = level->get<Enemy>(target)) {
        double xd = e->x - x, yd = e->bodyY() - y;
        double dd = std::sqrt(xd * xd + yd * yd);
        if (dd > 1e-5) {
            double k = damp(6, DT);
            xa += (xd / dd * 230 - xa) * k;
            ya += (yd / dd * 230 - ya) * k;
        }
    }
    x += xa * DT;
    y += ya * DT;
    if (rng.chance(0.5f)) {
        float dir = rng.range(0, TAU);
        level->add<Spark>(x, y, xa * -0.1 + std::cos(dir) * 8, ya * -0.1 + std::sin(dir) * 8, Color{0.4f, 0.75f, 1, 1}, 0.25f,
                          0.35f, false);
    }
    collide();
}

void Bolt::render(Screen &s) {
    s.color({0.45f, 0.8f, 1, 1});
    s.center(atlas::Bolt, xp, yp, 1.1f);
    s.color(col::white);
    s.center(atlas::Dot, xp, yp, 0.45f);
}

void Bolt::renderLight(Screen &s) { s.glow(xp, yp, 52, {0.06f, 0.20f, 0.36f, 1}); }

void Bolt::spent() { Spark::burst(*level, x, y, {0.45f, 0.8f, 1, 1}, 6, 60, true); }

Slash::Slash(Player &p, float side, float damage, float scale)
    : Projectile(p.x, p.bodyY(), 0, 0, 0.22f, damage, 90, 9999), player(p), side(side), scale(scale) {}

void Slash::tick() {
    if ((life -= DT) <= 0) return remove();
    x = player.x + side * 16 * scale;
    y = player.bodyY();
    collide();
}

bool Slash::touches(const Enemy &e) const {
    double xd = e.x - x, yd = e.bodyY() - y;
    float rx = 14 * scale + e.radius * 0.5f, ry = 24 * scale;
    return (xd * xd) / (rx * rx) + (yd * yd) / (ry * ry) <= 1;
}

void Slash::render(Screen &s) {
    float f = frac();
    s.color({1, 0.95f, 0.82f, std::pow(f, 0.6f)});
    s.sprite(atlas::Slash, xp, yp, 12, 24, side * scale * (0.8f + 0.4f * (1 - f)), scale);
    s.color(col::white);
}

void Slash::renderLight(Screen &s) { s.glow(xp, yp, 80 * scale, Color{0.16f, 0.15f, 0.11f, 1} * frac()); }

Axe::Axe(double x, double y, double xa, double ya, float damage, int pierce, float spin)
    : Projectile(x, y, xa, ya, 2.2f, damage, 60, pierce), spin(spin) {
    radius = 8;
}

void Axe::tick() {
    if ((life -= DT) <= 0) return remove();
    ya += 560 * DT;
    x += xa * DT;
    y += ya * DT;
    rot += spin * DT;
    collide();
}

void Axe::render(Screen &s) { s.center(atlas::tile(atlas::t::Axe), xp, yp, 1, rot); }

void Axe::renderLight(Screen &s) { s.glow(xp, yp, 44, {0.20f, 0.10f, 0.03f, 1}); }

void Arrow::tick() {
    if ((life -= DT) <= 0) return remove();
    x += xa * DT;
    y += ya * DT;
    collide();
}

void Arrow::render(Screen &s) {
    s.center(atlas::town(atlas::tt::Arrow), xp, yp, 1, float(std::atan2(ya, xa)) + PI / 4);
}

Strike::Strike(double x, double y, float damage, float radius) : damage(damage) {
    this->x = x, this->y = y, this->radius = radius;
    for (float &j : jag) j = rng.range(-7, 7);
}

void Strike::tick() {
    if (!struck) {
        struck = true;
        level->enemiesNear(x, y, radius + 24, [&](Enemy &e) {
            double xd = e.x - x, yd = e.bodyY() - y;
            float r = radius + e.radius;
            if (xd * xd + yd * yd < r * r) e.hurt(damage, e.x - x, e.y - y, 50);
        });
        Spark::burst(*level, x, y, {0.7f, 0.85f, 1, 1}, 10, 90, true);
    }
    if ((life -= DT) <= 0) remove();
}

void Strike::render(Screen &s) {
    float a = life / 0.3f;
    double x0 = xp, y0 = yp - 150;
    for (size_t i = 0; i < jag.size(); i++) {
        float k = float(i + 1) / float(jag.size());
        double x1 = xp, y1 = yp;
        if (i + 1 < jag.size()) x1 = xp + jag[i], y1 = yp - 150 * (1 - k);
        s.line(x0, y0, x1, y1, 3, {0.55f, 0.75f, 1, a * 0.7f});
        s.line(x0, y0, x1, y1, 1, col::white.alpha(a));
        x0 = x1, y0 = y1;
    }
    s.color({0.7f, 0.85f, 1, a * 0.6f});
    s.center(atlas::Ring, xp, yp, radius * 2 / atlas::Ring.w * (1.3f - 0.3f * a));
    s.color(col::white);
}

void Strike::renderLight(Screen &s) { s.glow(xp, yp, 160, Color{0.35f, 0.45f, 0.7f, 1} * (life / 0.3f)); }

void FirePool::tick() {
    if ((life -= DT) <= 0) return remove();
    if ((timer -= DT) <= 0) {
        timer = 0.4f;
        level->enemiesNear(x, y, radius + 24, [&](Enemy &e) {
            double xd = e.x - x, yd = e.y - y;
            float r = radius + e.radius;
            if (xd * xd + yd * yd < r * r) e.hurt(damage, 0, 0, 0);
        });
    }
    if (rng.chance(0.6f) && Spark::alive < 500) {
        float dir = rng.range(0, TAU), r = rng.range(0, radius);
        level->add<Spark>(x + std::cos(dir) * r, y + std::sin(dir) * r, rng.range(-6, 6), -rng.range(15, 35),
                          Color{1, rng.range(0.35f, 0.7f), 0.15f, 1}, rng.range(0.3f, 0.6f), rng.range(0.3f, 0.6f), false);
    }
}

void FirePool::render(Screen &s) {
    float fade = std::min(1.0f, life * 2) * std::min(1.0f, (maxLife - life) * 6);
    s.color({1, 0.45f, 0.12f, 0.35f * fade});
    s.center(atlas::Glow, xp, yp, radius * 2.4f / atlas::Glow.w);
    s.color({1, 0.6f, 0.2f, 0.25f * fade});
    s.center(atlas::Ring, xp, yp, radius * 2 / atlas::Ring.w, -life);
    s.color(col::white);
}

void FirePool::renderLight(Screen &s) {
    float flicker = 0.85f + 0.15f * std::sin(life * 23) * std::sin(life * 7);
    s.glow(xp, yp, radius * 4, Color{0.45f, 0.22f, 0.05f, 1} * (flicker * std::min(1.0f, life * 2)));
}

void Bomb::tick() {
    t += DT / 0.55f;
    float k = std::min(t, 1.0f);
    x = x0 + (x1 - x0) * k;
    y = y0 + (y1 - y0) * k;
    z = std::sin(k * PI) * 40;
    if (t < 1) return;
    level->enemiesNear(x, y, 28 + 24, [&](Enemy &e) {
        double xd = e.x - x, yd = e.y - y;
        float r = 28 + e.radius;
        if (xd * xd + yd * yd < r * r) e.hurt(damage, xd, yd, 90);
    });
    Spark::burst(*level, x, y, {1, 0.55f, 0.2f, 1}, 6, 110, true);
    level->add<Explosion>(x, y, 0);
    level->add<FirePool>(x, y, poolRadius, poolDamage, poolTime);
    Sound::explosion.play(0.5f);
    level->game.onExplosion();
    remove();
}

void Bomb::render(Screen &s) {
    s.sprite(atlas::Shadow, xp, std::floor(y) + 1, 8, 4, 0.6f, 0.6f);
    s.center(atlas::town(atlas::tt::Bomb), xp, yp - 6, 1, t * 9);
}

Sweep::Sweep(const Player &p, float reach, float damage)
    : player(p), reach(reach), damage(damage), angle(rng.range(0, TAU)), start(angle) {
    x = p.x, y = p.bodyY();
}

void Sweep::tick() {
    t += DT / 0.45f;
    angle = start + TAU * std::min(t, 1.0f);
    x = player.x + std::cos(angle) * reach;
    y = player.bodyY() + std::sin(angle) * reach;
    level->enemiesNear(x, y, 40, [&](Enemy &e) {
        double xd = e.x - x, yd = e.bodyY() - y;
        float r = 13 + e.radius;
        if (xd * xd + yd * yd >= r * r || std::find(hits.begin(), hits.end(), e.id) != hits.end()) return;
        hits.push_back(e.id);
        e.hurt(damage, e.x - player.x, e.y - player.y, 120);
    });
    if (t >= 1) remove();
}

void Sweep::render(Screen &s) {
    int xc = int(std::floor(player.x)), yc = int(std::floor(player.bodyY()));
    for (int i = 3; i >= 1; i--) {
        float a = angle - i * 0.28f;
        s.color({0.8f, 0.85f, 1, 0.15f * float(4 - i)});
        s.center(atlas::town(atlas::tt::Scythe), xc + std::cos(a) * reach, yc + std::sin(a) * reach, 1.2f, a + PI * 0.75f);
    }
    s.color(col::white);
    s.center(atlas::town(atlas::tt::Scythe), xp, yp, 1.2f, angle + PI * 0.75f);
}

void Sweep::renderLight(Screen &s) { s.glow(xp, yp, 60, {0.18f, 0.2f, 0.28f, 1}); }

void Orb::tick() {
    if ((life -= DT) <= 0) return remove();
    x += xa * DT;
    y += ya * DT;
    Player &p = *level->player;
    double xd = p.x - x, yd = p.bodyY() - y;
    if (!p.dead() && xd * xd + yd * yd < 7 * 7) {
        p.hurt(damage);
        Spark::burst(*level, x, y, {0.75f, 0.35f, 1, 1}, 6, 50, false);
        remove();
    }
}

void Orb::render(Screen &s) {
    s.color({0.75f, 0.35f, 1, 1});
    s.center(atlas::Bolt, xp, yp, 1.2f, life * 4);
    s.color(col::white);
    s.center(atlas::Dot, xp, yp, 0.4f);
}

void Orb::renderLight(Screen &s) { s.glow(xp, yp, 44, {0.25f, 0.08f, 0.35f, 1}); }
