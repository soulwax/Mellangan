#include "entity/enemy/Cultist.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/projectile/Orb.hpp"
#include "level/Level.hpp"

void Cultist::steer(double xn, double yn, double &xs, double &ys) {
    double w = std::cos(animT * 3) * 28;
    xs = xn * type.speed - yn * w;
    ys = yn * type.speed + xn * w;
}

void Cultist::tick() {
    Enemy::tick();
    if (removed || (castTimer -= DT) > 0) return;
    castTimer = rng.range(2.5f, 4.0f);
    Player &p = *level->player;
    double xd = p.x - x, yd = p.bodyY() - bodyY();
    double dd = std::sqrt(xd * xd + yd * yd);
    if (p.dead() || dd > 200 || dd < 1e-5) return;
    level->add<Orb>(x, bodyY(), xd / dd * 70, yd / dd * 70, 10 * damageMul);
    Sound::orb.play(0.3f);
}
