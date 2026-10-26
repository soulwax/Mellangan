#include "entity/Particle.hpp"

#include "level/Level.hpp"

void Spark::burst(Level &level, Vec2 at, Color c, int n, float speed, bool glow) {
    if (level.count<Spark>() > 700) return;
    for (int i = 0; i < n; i++)
        level.add<Spark>(at, rng.onCircle() * rng.range(0.3f, 1) * speed, c, rng.range(0.25f, 0.6f),
                         rng.range(0.3f, 0.7f), glow && i < 3);
}

void Spark::onAdded() { level->tag(this); }

void Spark::tick() {
    Particle::tick();
    vel *= std::exp(-5 * DT);
    pos += vel * DT;
}

void Spark::render(Screen &s) {
    s.color(color.alpha(color.a * frac()));
    s.center(atlas::Dot, pos, size * (0.5f + 0.5f * frac()));
    s.color(col::white);
}

void Spark::renderLight(Screen &s) {
    if (glow) s.glow(pos, 34, color * (0.5f * frac()));
}

void TextParticle::pop(Level &level, Vec2 at, int value, Color c) {
    if (level.count<TextParticle>() < 120) level.add<TextParticle>(at + Vec2{rng.range(-3, 3), -4}, value, c);
}

void TextParticle::onAdded() { level->tag(this); }

void TextParticle::tick() {
    Particle::tick();
    pos.y -= 18 * DT * frac();
}

void TextParticle::render(Screen &s) { s.number(value, pos, color.alpha(std::min(1.0f, frac() * 3))); }
