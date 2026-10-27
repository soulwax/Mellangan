#include "particle/debris/Debris.hpp"

#include "level/Level.hpp"
#include "particle/debris/BloodDebris.hpp"
#include "particle/debris/MeatDebris.hpp"
#include "particle/debris/SmokeDebris.hpp"

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
