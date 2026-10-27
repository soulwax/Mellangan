#include "entity/pickup/Pickup.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"

void Pickup::tick() {
    t += DT;
    z = -std::sin(t * 4 + x) * 1.5;
    Player &p = *level->player;
    if (p.dead()) return;
    double xd = p.x - x, yd = p.bodyY() - y;
    double d = std::sqrt(xd * xd + yd * yd);
    if (!magnet) {
        if (d < reach()) magnet = true, speed = -60;
        return;
    }
    speed += 650 * DT;
    if (d < 5 || d < speed * DT) return take(p), remove();
    x += xd / d * speed * DT;
    y += yd / d * speed * DT;
}

float Pickup::reach() const { return level->player->magnet; }
