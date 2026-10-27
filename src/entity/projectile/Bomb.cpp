#include "entity/projectile/Bomb.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/effect/FirePool.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Explosion.hpp"
#include "particle/Spark.hpp"

void Bomb::tick() {
    t += DT / 0.55f;
    float k = std::min(t, 1.0f);
    x = x0 + (x1 - x0) * k;
    y = y0 + (y1 - y0) * k;
    z = std::sin(k * PI) * 40;
    if (t < 1) return;
    level->enemiesNear(x, y, 28 + 24, [&](Enemy &e) {
        double xd = e.x - x, yd = e.y - y;
        float r = 28 + e.radius;
        if (xd * xd + yd * yd < r * r) e.hurt(damage, xd, yd, 90);
    });
    Spark::burst(*level, x, y, {1, 0.55f, 0.2f, 1}, 6, 110, true);
    level->add<Explosion>(x, y, 0);
    level->add<FirePool>(x, y, poolRadius, poolDamage, poolTime);
    Sound::explosion.play(0.5f);
    level->game.onExplosion();
    remove();
}

void Bomb::render(Screen &s) {
    s.sprite(atlas::Shadow, xp, std::floor(y) + 1, 8, 4, 0.6f, 0.6f);
    s.center(atlas::town(atlas::tt::Bomb), xp, yp - 6, 1, t * 9);
}
