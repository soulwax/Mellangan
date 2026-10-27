#include "particle/Debris.hpp"

#include "level/Level.hpp"

Debris::Debris(double x, double y, double z) {
    this->x = x, this->y = y, this->z = z;
    alive++;
    life = rng.irange(40, 59);
    double xn, yn, zn, dd;
    do {
        xn = rng.range(-1, 1), yn = rng.range(-1, 1), zn = rng.range(-1, 1);
        dd = xn * xn + yn * yn + zn * zn;
    } while (dd > 1 || dd < 1e-6);
    dd = std::sqrt(dd);
    xa = xn / dd, ya = yn / dd, za = zn / dd + 1;
}

bool Debris::room() { return alive < MAX; }

void Debris::gore(Level &level, double x, double y, double z, Color blood, bool meat) {
    for (int i = 0; i < 6 && room(); i++) level.add<BloodDebris>(x, y, z, blood);
    if (!meat) {
        for (int i = 0; i < 4 && room(); i++) level.add<SmokeDebris>(x, y, z);
        return;
    }
    for (int i = 0, n = rng.irange(1, 2); i < n && room(); i++) level.add<MeatDebris>(x, y, z, blood);
}

void Debris::tick() {
    if (--life < 0) return remove();
    if (z <= 1) xa *= 0.85, ya *= 0.85;
    else xa *= drag, ya *= drag;
    za -= gravity;
    attemptMove();
}

void Debris::attemptMove() {
    int steps = int(std::sqrt(xa * xa + ya * ya + za * za) + 1);
    for (int i = 0; i < steps; i++) {
        move(xa / steps, 0, 0);
        move(0, ya / steps, 0);
        move(0, 0, za / steps);
    }
}

void Debris::move(double xxa, double yya, double zza) {
    if (removed) return;
    double xn = x + xxa, yn = y + yya, zn = z + zza;
    if (zn < 0) {
        z = 0;
        return collide(xxa, yya, zza);
    }
    if (zn < 12 && level->blocks(xn, yn)) return collide(xxa, yya, zza);
    x = xn, y = yn, z = zn;
}

void Debris::collide(double xxa, double yya, double zza) {
    if (xxa != 0) xa *= -bounce;
    if (yya != 0) ya *= -bounce;
    if (zza != 0) za *= -bounce;
}

void Debris::shed(std::unique_ptr<Debris> d, double keep, double carry) {
    if (!room()) return;
    d->xa = d->xa * keep + xa * carry;
    d->ya = d->ya * keep + ya * carry;
    d->za = d->za * keep + za * carry;
    level->spawn(std::move(d));
}

void Debris::render(Screen &s) { s.particle(icon, xp, yp, color); }

void Debris::renderShadow(Screen &s) { s.rect(xp - 1, yp, 2, 1, {0, 0, 0, 0.45f}); }

BloodDebris::BloodDebris(double x, double y, double z, Color c) : Debris(x, y, z) {
    drag = 0.96, bounce = 0.1, icon = 1, color = c;
}

void BloodDebris::render(Screen &s) { s.rect(xp, yp, 1, 1, color * 0.75f); }

void BloodDebris::renderShadow(Screen &s) { s.rect(xp, yp, 1, 1, {0, 0, 0, 0.35f}); }

SmokeDebris::SmokeDebris(double x, double y, double z) : Debris(x, y, z) {
    lifeTime = life /= 2;
    drag = 0.92, gravity = -0.02, icon = rng.irange(8, 10);
}

void SmokeDebris::render(Screen &s) { s.particle(icon, xp, yp, {0.45f, 0.43f, 0.48f, 0.7f * float(life) / lifeTime}); }

FlameDebris::FlameDebris(double x, double y, double z) : Debris(x, y, z), lit(rng.chance(0.25f)) {
    maxLife = life /= 2;
    drag = 0.92, gravity = 0, icon = rng.irange(8, 10);
}

void FlameDebris::lick(Level &level, double x, double y, double z, float heat) {
    if (!room()) return;
    auto f = std::make_unique<FlameDebris>(x, y, z);
    f->xa *= 0.12 * heat, f->ya *= 0.12 * heat, f->za = rng.range(0.25f, 0.6f) * heat;
    f->gravity = -0.015, f->drag = 0.9, f->lit = false;
    f->maxLife = f->life = rng.irange(8, 16);
    level.spawn(std::move(f));
}

void FlameDebris::tick() {
    Debris::tick();
    if (!removed || noSmoke || !rng.chance(0.2f)) return;
    shed(std::make_unique<SmokeDebris>(x, y, z), 0.1, 1);
}

void FlameDebris::render(Screen &s) {
    icon = 8 * 2 + 3 - std::min(3, 4 * std::max(life, 0) / maxLife);
    Debris::render(s);
}

void FlameDebris::renderLight(Screen &s) {
    if (lit) s.glow(xp, yp, 26, Color{0.40f, 0.20f, 0.05f, 1} * (float(std::max(life, 0)) / maxLife));
}

MeatDebris::MeatDebris(double x, double y, double z, Color c) : Debris(x, y, z) { color = c; }

void MeatDebris::tick() {
    Debris::tick();
    if (!removed) shed(std::make_unique<BloodDebris>(x, y, z, color), 0.05, 0.5);
}

void MeatDebris::collide(double xxa, double yya, double zza) {
    if (za < -0.5) {
        for (int i = 0; i < 20 && room(); i++) {
            auto blood = std::make_unique<BloodDebris>(x, y, 0, color);
            blood->xa = blood->xa * 0.4 + xa * 0.5;
            blood->ya = blood->ya * 0.4 + ya * 0.5;
            blood->za = -za * 0.5;
            level->spawn(std::move(blood));
        }
    }
    Debris::collide(xxa, yya, zza);
}

HealDebris::HealDebris(double x, double y, double z, int kind) : Debris(x, y, z), lit(rng.chance(0.33f)) {
    life /= 2;
    drag = 0.92, gravity = 0, icon = 8 * 3 + kind;
}

void HealDebris::renderLight(Screen &s) {
    if (!lit) return;
    s.glow(xp, yp, 22, icon == 24 ? Color{0.30f, 0.06f, 0.06f, 1} : Color{0.10f, 0.10f, 0.35f, 1});
}

Explosion::Explosion(double x, double y, double z, int life) : life(life), maxLife(life) { this->x = x, this->y = y, this->z = z; }

void Explosion::tick() {
    if (--life < 0) return remove();
    int ps = life * 40 / maxLife + 1;
    double dd = double(maxLife - life) / maxLife + 0.2;
    for (int i = 0; i < ps && Debris::room(); i++) {
        double dir = rng.range(0, TAU), dist = rng.range(0, 6) * dd;
        double xx = x + std::cos(dir) * dist, yy = y + std::sin(dir) * dist, zz = z + rng.range(0, 10);
        auto fd = std::make_unique<FlameDebris>(xx, yy, zz);
        if (rng.chance(0.5f)) fd->life = fd->maxLife / 2;
        fd->xa = fd->xa * 0.1 + (xx - x) * 0.5;
        fd->ya = fd->ya * 0.1 + (yy - y) * 0.5;
        fd->za *= 0.1;
        fd->gravity = 0.10;
        level->spawn(std::move(fd));
    }
}
