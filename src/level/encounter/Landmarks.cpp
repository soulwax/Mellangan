#include "level/encounter/Common.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/pickup/Gem.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/FlameDebris.hpp"

namespace encounter {

namespace {

const char *const LORE[] = {
    "THE STONE REMEMBERS A SKY WITH TWO MOONS",
    "THE STONE NAMES THE SHRINES THAT NEVER WOKE",
    "THE STONE TELLS OF A DAWN THAT WAS EARNED",
    "THE STONE SHOWS THE LAND AS THE GIANTS SAW IT",
};

bool unlitShrine(const Structure *s, const void *ctx) {
  return s->kind == StructureKind::Shrine &&
         !static_cast<const Level *>(ctx)->night.isLit(*s);
}

} // namespace

void ancientTree(Level *level, Site s) {
  Player &p = *level->player;
  p.maxHp += 15;
  p.heal(p.maxHp);
  Spark::burst(*level, s->centerX(), footY(s) - 40, {0.95f, 0.85f, 0.4f, 1}, 24,
               90, true);
  for (int i = 0; i < 14; i++)
    drift(level, s->centerX() + rng.range(-24, 24), footY(s) - rng.range(30, 60),
          atlas::pt::Leaf, {1, 1, 1, 1}, 12);
  level->game.announce("THE TREE RENEWS YOU: +15 MAX HEALTH");
  Sound::potion.play(0.7f, 0);
}

void leaves(Level *level, Site s) {
  if (rng.chance(0.05f))
    drift(level, s->centerX() + rng.range(-26, 26), footY(s) - rng.range(36, 60),
          atlas::pt::Leaf, {1, 1, 1, 0.9f}, 14);
  if (rng.chance(0.08f))
    level->add<Spark>(s->centerX() + rng.range(-28, 28),
                      footY(s) - rng.range(20, 64), rng.range(-4, 4),
                      rng.range(-6, 2), Color{0.95f, 0.9f, 0.45f, 1},
                      rng.range(1.5f, 2.5f), 0.3f, true);
}

void fallenStar(Level *level, Site s) {
  double x = s->centerX(), y = footY(s) - 10;
  level->add<Shockwave>(x, y, 6, 110, 0.5f, Color{1, 0.9f, 0.55f, 1})->spokes = 10;
  quake(level, x, y, 110, 60 + level->time * 0.1f, 160);
  for (int i = 0; i < 12; i++)
    Gem::drop(*level, x + rng.range(-20, 20), y + rng.range(-6, 14), 4);
  level->add<Chest>(x, footY(s) + 14);
  Sound::nova.play(0.6f, 0);
}

void starlight(Level *level, Site s) {
  if (rng.chance(0.12f))
    level->add<Spark>(s->centerX() + rng.range(-14, 14), footY(s) - rng.range(8, 34),
                      rng.range(-3, 3), rng.range(-22, -10),
                      Color{1, 0.9f, 0.55f, 1}, rng.range(1.2f, 2), 0.3f, true);
}

void colossus(Level *level, Site s) {
  int n = 6 + int(level->time / 60);
  for (int i = 0; i < n; i++) {
    double dir = TAU * i / n;
    level->spawner.spawn(EnemyKind::Skeleton, s->centerX() + std::cos(dir) * 90,
                         footY(s) + std::sin(dir) * 70);
  }
  level->add<Chest>(s->centerX() + 30, footY(s) + 12);
  Sound::raise.play(0.6f, 0);
}

void bones(Level *level, Site s) {
  if (rng.chance(0.03f))
    drift(level, s->centerX() + rng.range(-20, 20), footY(s) - rng.range(10, 30),
          atlas::pt::Puff, {0.7f, 0.78f, 0.9f, 0.35f}, -4);
}

void monolith(Level *level, Site s) {
  level->chart.unveil(s->centerX(), footY(s), 30);
  level->game.announce(LORE[s->seed % std::size(LORE)]);
  level->add<Shockwave>(s->centerX(), footY(s) - 20, 4, 90, 0.6f,
                        Color{0.35f, 0.75f, 1, 1});
  Sound::magnet.play(0.6f, 0);
}

void runes(Level *level, Site s) {
  if (rng.chance(0.1f))
    level->add<Spark>(s->centerX() + rng.range(-5, 5), footY(s) - rng.range(10, 56),
                      0, rng.range(-8, -3), Color{0.45f, 0.85f, 1, 1},
                      rng.range(0.8f, 1.6f), 0.3f, true);
}

void spire(Level *level, Site s) {
  Player &p = *level->player;
  p.skillTime = 0, p.might += 0.05f;
  double x = s->centerX(), y = footY(s) - 20;
  level->add<Shockwave>(x, y, 6, 120, 0.5f, Color{0.75f, 0.55f, 1, 1})->spokes = 8;
  quake(level, x, y, 120, 40 + level->time * 0.08f, 120);
  level->game.announce("YOUR POWER RINGS: SKILL READY, +5% MIGHT");
  Sound::nova.play(0.5f, 0);
}

void shimmer(Level *level, Site s) {
  if (rng.chance(0.08f))
    level->add<Spark>(s->centerX() + rng.range(-18, 18), footY(s) - rng.range(10, 58),
                      0, rng.range(-6, -2), Color{0.8f, 0.6f, 1, 1},
                      rng.range(1, 2), 0.3f, true);
}

void mushroom(Level *level, Site s) {
  Player &p = *level->player;
  p.regen += 0.4f;
  level->enemiesNear(s->centerX(), footY(s), 140, [&](Enemy &e) {
    e.afflict(StatusKind::Poison, 6, 5);
  });
  for (int i = 0; i < 16; i++)
    drift(level, s->centerX() + rng.range(-30, 30), footY(s) - rng.range(20, 50),
          atlas::pt::Spore, {0.95f, 0.7f, 0.85f, 0.9f}, -8);
  level->game.announce("THE SPORES MEND YOU: +0.4 REGEN/S");
  Sound::potion.play(0.6f, 0);
}

void spores(Level *level, Site s) {
  if (rng.chance(0.06f))
    drift(level, s->centerX() + rng.range(-24, 24), footY(s) - rng.range(34, 56),
          atlas::pt::Spore, {0.95f, 0.75f, 0.9f, 0.9f}, -6);
}

void wreck(Level *level, Site s) {
  level->add<Chest>(s->centerX() - 24, footY(s) + 12);
  level->add<Chest>(s->centerX() + 24, footY(s) + 12);
  for (int i = 0; i < 10; i++)
    Gem::drop(*level, s->centerX() + rng.range(-30, 30), footY(s) + rng.range(0, 16), 3);
  Sound::chest.play(0.6f, 0);
}

void gate(Level *level, Site) {
  Player &p = *level->player;
  Structure target;
  if (!world::nearestStructure(p.x, p.y, 40, target, unlitShrine, level)) {
    level->game.announce("THE GATE IS SILENT");
    return;
  }
  level->add<Shockwave>(p.x, p.bodyY(), 4, 60, 0.4f, Color{0.6f, 0.4f, 1, 1});
  p.x = target.centerX(), p.y = (target.y + target.h + 1) * 16.0;
  p.invuln = std::max(p.invuln, 1.0f);
  level->game.snapCamera();
  level->add<Shockwave>(p.x, p.bodyY(), 4, 80, 0.5f, Color{0.6f, 0.4f, 1, 1})->spokes = 10;
  level->game.announce("THE GATE CARRIES YOU TO A SLEEPING SHRINE");
  Sound::magnet.play(0.7f, 0);
}

void swirl(Level *level, Site s) {
  if (rng.chance(0.1f))
    level->add<Spark>(s->centerX() + rng.range(-8, 8), footY(s) - rng.range(14, 40),
                      rng.range(-6, 6), rng.range(-6, 6), Color{0.65f, 0.45f, 1, 1},
                      rng.range(0.8f, 1.4f), 0.3f, true);
}

void forge(Level *level, Site s) {
  Player &p = *level->player;
  std::vector<Upgrade *> arms;
  for (auto &u : p.upgrades)
    if (u->weapon && u->level > 0 && !u->maxed())
      arms.push_back(u.get());
  if (arms.empty()) {
    level->game.announce("THE FORGE HAS NOTHING TO TEMPER");
    return;
  }
  Upgrade &u = *arms[rng.irange(0, int(arms.size()) - 1)];
  u.levelUp(p);
  for (int i = 0; i < 18; i++)
    FlameDebris::lick(*level, s->centerX() + rng.range(-14, 14), footY(s), 8, 1.2f);
  level->game.announce(std::string("THE FORGE TEMPERS ") + u.name);
  Sound::axe.play(0.8f, 0);
}

void coals(Level *level, Site s) {
  if (rng.chance(0.2f))
    FlameDebris::lick(*level, s->centerX() + rng.range(-10, 10), footY(s) - 2, 4, 0.8f);
}

void frozen(Level *level, Site s) {
  Player &p = *level->player;
  p.armor += 0.5f;
  p.traits.add(Trait::Frosted);
  level->enemiesNear(p.x, p.y, 220, [&](Enemy &e) {
    e.afflict(StatusKind::Chill, 6, 0.85f);
  });
  level->add<Shockwave>(s->centerX(), footY(s) - 20, 6, 200, 0.7f,
                        Color{0.75f, 0.9f, 1, 1})->spokes = 12;
  level->game.announce("THE ICE CLAIMS YOU: +0.5 ARMOUR, A FREEZING TRAIL");
  Sound::nova.play(0.5f, 0);
}

void frost(Level *level, Site s) {
  if (rng.chance(0.05f))
    drift(level, s->centerX() + rng.range(-20, 20), footY(s) - rng.range(20, 60),
          atlas::pt::Snowflake, {1, 1, 1, 0.8f}, 8);
}

} // namespace encounter
