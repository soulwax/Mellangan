#include "entity/effect/Sweep.hpp"

#include <algorithm>

#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"

Sweep::Sweep(const Player &p, float reach, float damage)
    : player(p), reach(reach), damage(damage), angle(rng.range(0, TAU)), start(angle) {
    x = p.x, y = p.bodyY();
}

void Sweep::tick() {
    t += DT / 0.45f;
    angle = start + TAU * std::min(t, 1.0f);
    x = player.x + std::cos(angle) * reach;
    y = player.bodyY() + std::sin(angle) * reach;
    level->enemiesNear(x, y, 40, [&](Enemy &e) {
        double xd = e.x - x, yd = e.bodyY() - y;
        float r = 13 + e.radius;
        if (xd * xd + yd * yd >= r * r || std::find(hits.begin(), hits.end(), e.id) != hits.end()) return;
        hits.push_back(e.id);
        e.hurt(damage, e.x - player.x, e.y - player.y, 120);
    });
    if (t >= 1) remove();
}

void Sweep::render(Screen &s) {
    int xc = int(std::floor(player.x)), yc = int(std::floor(player.bodyY()));
    for (int i = 3; i >= 1; i--) {
        float a = angle - i * 0.28f;
        s.color({0.8f, 0.85f, 1, 0.15f * float(4 - i)});
        s.center(atlas::town(atlas::tt::Scythe), xc + std::cos(a) * reach, yc + std::sin(a) * reach, 1.2f, a + PI * 0.75f);
    }
    s.color(col::white);
    s.center(atlas::town(atlas::tt::Scythe), xp, yp, 1.2f, angle + PI * 0.75f);
}

void Sweep::renderLight(Screen &s) { s.glow(xp, yp, 60, {0.18f, 0.2f, 0.28f, 1}); }
