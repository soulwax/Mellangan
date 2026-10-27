#include "entity/enemy/Scorpion.hpp"

void Scorpion::steer(double xn, double yn, double &xs, double &ys) {
    double v = type.speed * (0.5f + std::max(0.0f, std::sin(animT * 4)) * 1.2f);
    xs = xn * v;
    ys = yn * v;
}
