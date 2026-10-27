#include "level/biome/Snow.hpp"

#include "level/Level.hpp"
#include "particle/debris/SnowDebris.hpp"

namespace f = feature;

Snow::Snow()
    : Biome({"SNOW", {0.85f, 0.92f, 1.0f, 1}, {0.96f, 0.98f, 1.04f, 1}, 0.38f, 0, 26, 55, {10, 10, 20, 10, 10, 40},
     foe(EnemyKind::Bat) | foe(EnemyKind::Ghost) | foe(EnemyKind::Knight),
     {{&f::snowPine, 25, 280}, {&f::snowOak, 8, 120}, {&f::snowBush, 6, 50}, {&f::deadPine, 2, 15},
      {&f::crystal, 3, 0}, {&f::log, 3, 0}, {&f::snowyFir, 12, 140}, {&f::frostBirch, 4, 40}, {&f::snowyBoulder, 2, 0}, {&f::iceChunk, 2, 0}, {&f::frostedFern, 2, 0}}, &f::frostTuft}) {}

Sprite Snow::grass(int kind) const {
    Sprite s = Biome::grass(kind);
    s.y += 264;
    return s;
}

Sprite Snow::dirtRim(int offset) const { return atlas::winter(atlas::tt::DirtTopLeft + offset); }

Sprite Snow::frost(Sprite ground) const {
    if (ground.x >= 216 && ground.y < 72) ground.y += 264;
    return ground;
}

void Snow::weather(Level &level, double x, double y) const {
    double wind = 0.25 * std::sin(level.time * 0.3);
    for (int i = 0; i < 3 && Debris::room(); i++)
        level.add<SnowDebris>(x + rng.range(-300, 300), y + rng.range(-180, 200), rng.range(30, 90), wind);
}
