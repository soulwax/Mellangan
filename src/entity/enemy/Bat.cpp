#include "entity/enemy/Bat.hpp"

void Bat::steer(double xn, double yn, double &xs, double &ys) {
    double w = std::sin(animT * 5) * 34;
    xs = xn * type.speed - yn * w;
    ys = yn * type.speed + xn * w;
}
