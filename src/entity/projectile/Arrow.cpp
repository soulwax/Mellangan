#include "entity/projectile/Arrow.hpp"

void Arrow::tick() {
    if ((life -= DT) <= 0) return remove();
    x += xa * DT;
    y += ya * DT;
    collide();
}

void Arrow::render(Screen &s) {
    s.center(atlas::town(atlas::tt::Arrow), xp, yp, 1, float(std::atan2(ya, xa)) + PI / 4);
}
