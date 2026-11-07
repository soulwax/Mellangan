#include "level/Level.hpp"

#include <cstring>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Hazard.hpp"
#include "level/World.hpp"
#include "particle/Spark.hpp"

namespace {

constexpr Color EMBER{1, 0.45f, 0.12f, 1}, BONE{0.82f, 0.86f, 0.74f, 1};

float threatAt(Level &level, double x, double y) {
  return level.rules.threat ? level.rules.threat(x, y) : level.time;
}

void erupt(Level &level, int tx, int ty) {
  int hot[21 * 17], n = 0;
  for (int y = -8; y <= 8; y++)
    for (int x = -10; x <= 10; x++)
      if (world::isLava(tx + x, ty + y))
        hot[n++] = (y + 8) * 21 + x + 10;
  if (!n)
    return;
  int pick = hot[rng.irange(0, n - 1)];
  double cx = (tx + pick % 21 - 10) * 16 + 8.0, cy = (ty + pick / 21 - 8) * 16 + 8.0;
  float damage = level.game.balance.enemyDamage * (8 + threatAt(level, cx, cy) / 40);
  Blast *b = level.add<Blast>(cx, cy, damage, true);
  b->tone = EMBER, b->radius = 34, b->by = "an erupting vent";
}

void wake(Level &level, int tx, int ty) {
  int graves[15 * 13], n = 0;
  Prop p;
  for (int y = -6; y <= 6; y++)
    for (int x = -7; x <= 7; x++)
      if (std::abs(x) + std::abs(y) >= 4 && world::propAt(tx + x, ty + y, p) && p.kind &&
          std::strcmp(p.kind, "grave") == 0)
        graves[n++] = (y + 6) * 15 + x + 7;
  for (int raised = 0, tries = 0; raised < 2 && n > 0 && tries < 4; tries++) {
    int k = rng.irange(0, n - 1), g = graves[k];
    graves[k] = graves[--n];
    double sx = (tx + g % 15 - 7) * 16 + 8.0, sy = (ty + g / 15 - 6) * 16 + 28.0;
    if (level.rockAt(sx, sy - 2) || !level.spawner.spawn(EnemyKind::Skeleton, sx, sy, true))
      continue;
    Spark::burst(level, sx, sy - 4, BONE, 6, 40, false);
    raised++;
  }
}

} // namespace

void Level::caveRules() {
  if (world::layer() != world::Layer::Caves || player->dead())
    return;
  int tx = world::tileOf(player->x), ty = world::tileOf(player->y);
  CaveBiome here = world::caveBiomeAt(tx, ty);
  if (here == CaveBiome::Vents && (ventTime -= DT) <= 0)
    ventTime = rng.range(2.5f, 5.0f), erupt(*this, tx, ty);
  if (here == CaveBiome::Catacombs && (raiseTime -= DT) <= 0)
    raiseTime = rng.range(9.0f, 14.0f), wake(*this, tx, ty);
}
