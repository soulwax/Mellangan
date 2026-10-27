#include "level/biome/Mire.hpp"

#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace f = feature;

Mire::Mire()
    : Biome({"MIRE", {0.45f, 0.55f, 0.3f, 1}, {0.86f, 0.92f, 0.8f, 1}, 0.12f, 0, 34, 50, {5, 15, 15, 25, 40, 0},
     foe(EnemyKind::Slime) | foe(EnemyKind::Spider) | foe(EnemyKind::Cultist),
     {{&f::willow, 14, 200}, {&f::reeds, 16, 60}, {&f::mossPine, 6, 90}, {&f::deadPine, 3, 30},
      {&f::toadstools, 4, 0}, {&f::blueShrooms, 3, 0}, {&f::log, 4, 0}, {&f::spiritPine, 1, 10}, {&f::cattails, 8, 20}, {&f::fern, 3, 0}, {&f::mossyBoulder, 1, 0}, {&f::mushroomTree, 1, 6}, {&f::weepingWillow, 8, 100}, {&f::glowshrooms, 2, 0}, {&f::mossyLog, 3, 0}, {&f::driftwood, 1, 0}, {&f::callaLilies, 1, 0}}, &f::bogTuft}) {}

Sprite Mire::dirtRim(int offset) const { return atlas::mudRim(offset); }

void Mire::weather(Level &level, double x, double y) const {
    if (Spark::alive > 500 || rng.unit() > 0.25f) return;
    level.add<Spark>(x + rng.range(-260, 260), y + rng.range(-160, 180), rng.range(-6, 6), rng.range(-6, 6),
                     Color{0.75f, 1.0f, 0.35f, 1}, rng.range(1.2f, 2.4f), 0.35f, true);
}
