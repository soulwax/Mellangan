#include "entity/projectile/Slash.hpp"

#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/projectile/Projectile.hpp"

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
