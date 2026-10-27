#include "entity/enemy/Ghost.hpp"

#include "entity/enemy/Enemy.hpp"

void Ghost::steer(double xn, double yn, double &xs, double &ys) {
    double v = type.speed * (0.8f + 0.4f * std::sin(animT * 2));
    xs = xn * v;
    ys = yn * v;
}

void Ghost::renderLight(Screen &s) {
    Enemy::renderLight(s);
    s.glow(xp, yp - 7 * scale, 40, {0.06f, 0.10f, 0.18f, 1});
}
