#include "entity/projectile/Axe.hpp"

#include "entity/projectile/Projectile.hpp"

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
