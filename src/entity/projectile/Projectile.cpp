#include "entity/projectile/Projectile.hpp"

#include <algorithm>

#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"

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
