#include "particle/Spark.hpp"

#include "level/Level.hpp"
#include "particle/Particle.hpp"

void Spark::burst(Level &level, double x, double y, Color c, int n, float speed, bool glow) {
    if (alive > 700) return;
    for (int i = 0; i < n; i++) {
        float dir = rng.range(0, TAU), v = rng.range(0.3f, 1) * speed;
        level.add<Spark>(x, y, std::cos(dir) * v, std::sin(dir) * v, c, rng.range(0.25f, 0.6f), rng.range(0.3f, 0.7f),
                         glow && i < 3);
    }
}

void Spark::tick() {
    Particle::tick();
    double k = std::exp(-5 * DT);
    xa *= k, ya *= k;
    x += xa * DT;
    y += ya * DT;
}

void Spark::render(Screen &s) {
    s.color(color.alpha(color.a * frac()));
    s.center(sprite, xp, yp, size * (0.5f + 0.5f * frac()));
    s.color(col::white);
}

void Spark::renderLight(Screen &s) {
    if (glow) s.glow(xp, yp, 34, color * (0.3f * frac()));
}
