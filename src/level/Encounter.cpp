#include "level/Encounter.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/SupplyBarrel.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/pickup/Magnet.hpp"
#include "entity/pickup/Potion.hpp"
#include "level/Level.hpp"
#include "particle/Drift.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"
#include "entity/pickup/Gem.hpp"
#include "item/Upgrade.hpp"
#include "Game.hpp"
#include "particle/debris/FlameDebris.hpp"
#include "particle/debris/SmokeDebris.hpp"

namespace {

double atX(const Structure *s, int lx) { return (s->x + lx) * 16 + 8.0; }
double atY(const Structure *s, int ly) { return (s->y + ly) * 16 + 12.0; }

void cottage(Level *level, const Structure *s) {
  level->add<Potion>(atX(s, 2), atY(s, 4));
}

void graveyard(Level *level, const Structure *s) {
  int n = 5 + int(level->time / 40);
  for (int i = 0; i < n; i++) {
    double dir = TAU * i / n;
    level->spawner.spawn(EnemyKind::Ghost, s->centerX() + std::cos(dir) * 110,
                         s->centerY() + std::sin(dir) * 110);
  }
  if (level->time >= 40)
    level->spawner.spawn(EnemyKind::Necromancer, s->centerX(), s->centerY());
  Sound::horde.play(0.5f, 0.1f);
}

void shrine(Level *, const Structure *) {}

void camp(Level *level, const Structure *s) {
  auto b = std::make_unique<SupplyBarrel>();
  b->x = atX(s, s->w / 2 + 1);
  b->y = atY(s, s->h / 2 + 1);
  level->add(std::move(b));
}

void ruin(Level *level, const Structure *s) {
  double x = atX(s, s->w / 2), y = atY(s, 2);
  if (s->seed & 1)
    level->add<Chest>(x, y);
  else
    level->add<Magnet>(x, y);
}

void castle(Level *level, const Structure *s) {
  double x = s->centerX(), y = (s->y + s->h) * 16 + 4.0;
  if (s->seed & 1)
    level->add<Chest>(x, (s->y + 4) * 16 + 8.0);
  else
    level->add<Chest>(x, y);
  for (int i = 0, n = 3 + int(level->time / 60); i < n; i++) {
    double a = PI * (0.15 + 0.7 * i / std::max(1, n - 1));
    level->spawner.spawn(EnemyKind::Knight, x + std::cos(a) * 140,
                         y + std::sin(a) * 120);
  }
  Sound::horde.play(0.6f, 0);
}

void stones(Level *level, const Structure *s) {
  Player &p = *level->player;
  p.heal(p.maxHp);
  for (int i = 0; i < 40; i++) {
    float a = TAU * i / 40;
    level->add<Spark>(s->centerX(), s->centerY(), std::cos(a) * 90,
                      std::sin(a) * 90, Color{0.5f, 0.95f, 1, 1}, 0.7f, 0.5f,
                      false);
  }
  Sound::levelUp.play(0.5f, 0);
}

void mine(Level *level, const Structure *s) {
  double x = (s->x + 3) * 16 + 8.0, y = (s->y + 2) * 16 + 4.0;
  level->add<Chest>((s->x + 3) * 16 + 8.0, (s->y + 5) * 16 + 8.0);
  for (int i = 0, n = 4 + int(level->time / 45); i < n; i++)
    level->spawner.spawn(i % 3 ? EnemyKind::Spider : EnemyKind::Bat,
                         x + rng.range(-10, 10), y + rng.range(-4, 4));
  Sound::horde.play(0.5f, 0.1f);
}

void hum(Level *level, const Structure *s) {
  if (Spark::alive > 500 || !rng.chance(0.25f))
    return;
  level->add<Spark>(s->centerX() + rng.range(-6, 6), s->centerY() - 6,
                    rng.range(-3, 3), rng.range(-24, -12),
                    Color{0.5f, 0.95f, 1, 1}, rng.range(0.6f, 1.2f), 0.3f,
                    true);
}

void smoke(Level *level, double x, double y, double z) {
  if (!Debris::room())
    return;
  auto puff = std::make_unique<SmokeDebris>(x, y, z);
  puff->xa *= 0.15, puff->ya *= 0.05, puff->za = 0.3, puff->life *= 2;
  level->spawn(std::move(puff));
}

void hearth(Level *level, const Structure *s) {
  int lx, height;
  if (rng.chance(0.12f) && cottageChimney(s, &lx, &height))
    smoke(level, (s->x + lx) * 16 + 8.0, (s->y + 2) * 16 + 15.0, height + 4);
}

void campfire(Level *level, const Structure *s) {
  int c = s->w / 2;
  double x = (s->x + c) * 16 + 8.0, y = (s->y + c + 1) * 16 + 1.0;
  for (int i = 0; i < 2; i++)
    FlameDebris::lick(*level, x + rng.range(-3, 3), y, 5, 1.2f);
  if (rng.chance(0.15f))
    smoke(level, x, y, 14);
}

void braziers(Level *level, const Structure *s) {
  int gate = s->w / 2 - 1;
  double y = (s->y + s->h - 1) * 16 + 4.0;
  if (rng.chance(0.6f))
    FlameDebris::lick(*level, (s->x + gate) * 16 - 2.0, y, 6, 0.8f);
  if (rng.chance(0.6f))
    FlameDebris::lick(*level, (s->x + gate + 2) * 16 + 2.0, y, 6, 0.8f);
}

double footY(const Structure *s) { return (s->y + 5) * 16.0; }

void drift(Level *level, double x, double y, int icon, Color c, float rise) {
  if (!Drift::room())
    return;
  Drift *d = level->add<Drift>(x, y, rng.range(-6, 6), rise, icon, c,
                               rng.range(2.5f, 4));
  d->sway = 8, d->scale = 0.8f, d->drag = 0.4f;
}

void ancientTree(Level *level, const Structure *s) {
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

void leaves(Level *level, const Structure *s) {
  if (rng.chance(0.05f))
    drift(level, s->centerX() + rng.range(-26, 26), footY(s) - rng.range(36, 60),
          atlas::pt::Leaf, {1, 1, 1, 0.9f}, 14);
  if (rng.chance(0.08f))
    level->add<Spark>(s->centerX() + rng.range(-28, 28),
                      footY(s) - rng.range(20, 64), rng.range(-4, 4),
                      rng.range(-6, 2), Color{0.95f, 0.9f, 0.45f, 1},
                      rng.range(1.5f, 2.5f), 0.3f, true);
}

void fallenStar(Level *level, const Structure *s) {
  double x = s->centerX(), y = footY(s) - 10;
  level->add<Shockwave>(x, y, 6, 110, 0.5f, Color{1, 0.9f, 0.55f, 1})->spokes = 10;
  level->enemiesNear(x, y, 120, [&](Enemy &e) {
    double xd = e.x - x, yd = e.y - y;
    if (xd * xd + yd * yd < 110 * 110)
      e.hurt(60 + level->time * 0.1f, xd, yd, 160);
  });
  for (int i = 0; i < 12; i++)
    Gem::drop(*level, x + rng.range(-20, 20), y + rng.range(-6, 14), 4);
  level->add<Chest>(x, footY(s) + 14);
  Sound::nova.play(0.6f, 0);
}

void starlight(Level *level, const Structure *s) {
  if (rng.chance(0.12f))
    level->add<Spark>(s->centerX() + rng.range(-14, 14), footY(s) - rng.range(8, 34),
                      rng.range(-3, 3), rng.range(-22, -10),
                      Color{1, 0.9f, 0.55f, 1}, rng.range(1.2f, 2), 0.3f, true);
}

void colossus(Level *level, const Structure *s) {
  int n = 6 + int(level->time / 60);
  for (int i = 0; i < n; i++) {
    double dir = TAU * i / n;
    level->spawner.spawn(EnemyKind::Skeleton, s->centerX() + std::cos(dir) * 90,
                         footY(s) + std::sin(dir) * 70);
  }
  level->add<Chest>(s->centerX() + 30, footY(s) + 12);
  Sound::raise.play(0.6f, 0);
}

void bones(Level *level, const Structure *s) {
  if (rng.chance(0.03f))
    drift(level, s->centerX() + rng.range(-20, 20), footY(s) - rng.range(10, 30),
          atlas::pt::Puff, {0.7f, 0.78f, 0.9f, 0.35f}, -4);
}

const char *const LORE[] = {
    "THE STONE REMEMBERS A SKY WITH TWO MOONS",
    "THE STONE NAMES THE SHRINES THAT NEVER WOKE",
    "THE STONE TELLS OF A DAWN THAT WAS EARNED",
    "THE STONE SHOWS THE LAND AS THE GIANTS SAW IT",
};

void monolith(Level *level, const Structure *s) {
  level->chart.unveil(s->centerX(), footY(s), 30);
  level->game.announce(LORE[s->seed % std::size(LORE)]);
  level->add<Shockwave>(s->centerX(), footY(s) - 20, 4, 90, 0.6f,
                        Color{0.35f, 0.75f, 1, 1});
  Sound::magnet.play(0.6f, 0);
}

void runes(Level *level, const Structure *s) {
  if (rng.chance(0.1f))
    level->add<Spark>(s->centerX() + rng.range(-5, 5), footY(s) - rng.range(10, 56),
                      0, rng.range(-8, -3), Color{0.45f, 0.85f, 1, 1},
                      rng.range(0.8f, 1.6f), 0.3f, true);
}

void spire(Level *level, const Structure *s) {
  Player &p = *level->player;
  p.skillTime = 0, p.might += 0.05f;
  double x = s->centerX(), y = footY(s) - 20;
  level->add<Shockwave>(x, y, 6, 120, 0.5f, Color{0.75f, 0.55f, 1, 1})->spokes = 8;
  level->enemiesNear(x, y, 130, [&](Enemy &e) {
    double xd = e.x - x, yd = e.y - y;
    if (xd * xd + yd * yd < 120 * 120)
      e.hurt(40 + level->time * 0.08f, xd, yd, 120);
  });
  level->game.announce("YOUR POWER RINGS: SKILL READY, +5% MIGHT");
  Sound::nova.play(0.5f, 0);
}

void shimmer(Level *level, const Structure *s) {
  if (rng.chance(0.08f))
    level->add<Spark>(s->centerX() + rng.range(-18, 18), footY(s) - rng.range(10, 58),
                      0, rng.range(-6, -2), Color{0.8f, 0.6f, 1, 1},
                      rng.range(1, 2), 0.3f, true);
}

void mushroom(Level *level, const Structure *s) {
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

void spores(Level *level, const Structure *s) {
  if (rng.chance(0.06f))
    drift(level, s->centerX() + rng.range(-24, 24), footY(s) - rng.range(34, 56),
          atlas::pt::Spore, {0.95f, 0.75f, 0.9f, 0.9f}, -6);
}

void wreck(Level *level, const Structure *s) {
  level->add<Chest>(s->centerX() - 24, footY(s) + 12);
  level->add<Chest>(s->centerX() + 24, footY(s) + 12);
  for (int i = 0; i < 10; i++)
    Gem::drop(*level, s->centerX() + rng.range(-30, 30), footY(s) + rng.range(0, 16), 3);
  Sound::chest.play(0.6f, 0);
}

bool unlitShrine(const Structure *s, const void *ctx) {
  return s->kind == StructureKind::Shrine &&
         !static_cast<const Level *>(ctx)->night.isLit(*s);
}

void gate(Level *level, const Structure *s) {
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

void swirl(Level *level, const Structure *s) {
  if (rng.chance(0.1f))
    level->add<Spark>(s->centerX() + rng.range(-8, 8), footY(s) - rng.range(14, 40),
                      rng.range(-6, 6), rng.range(-6, 6), Color{0.65f, 0.45f, 1, 1},
                      rng.range(0.8f, 1.4f), 0.3f, true);
}

void forge(Level *level, const Structure *s) {
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

void coals(Level *level, const Structure *s) {
  if (rng.chance(0.2f))
    FlameDebris::lick(*level, s->centerX() + rng.range(-10, 10), footY(s) - 2, 4, 0.8f);
}

void frozen(Level *level, const Structure *s) {
  Player &p = *level->player;
  p.armor += 0.5f;
  level->enemiesNear(p.x, p.y, 220, [&](Enemy &e) {
    e.afflict(StatusKind::Chill, 6, 0.85f);
  });
  level->add<Shockwave>(s->centerX(), footY(s) - 20, 6, 200, 0.7f,
                        Color{0.75f, 0.9f, 1, 1})->spokes = 12;
  level->game.announce("THE ICE HARDENS YOUR SKIN: +0.5 ARMOUR");
  Sound::nova.play(0.5f, 0);
}

void frost(Level *level, const Structure *s) {
  if (rng.chance(0.05f))
    drift(level, s->centerX() + rng.range(-20, 20), footY(s) - rng.range(20, 60),
          atlas::pt::Snowflake, {1, 1, 1, 0.8f}, 8);
}

} // namespace

const EncounterDef ENCOUNTERS[int(StructureKind::Count)] = {
    {"A WARM HEARTH", cottage, hearth},
    {"THE DEAD STIR", graveyard, nullptr},
    {"STAND AND KINDLE THE SHRINE", shrine, nullptr},
    {"AN ABANDONED CAMP", camp, campfire},
    {"SOMETHING GLINTS IN THE RUBBLE", ruin, nullptr},
    {"THE GARRISON WAKES", castle, braziers},
    {"THE STONES HUM", stones, hum},
    {"SOMETHING STIRS BELOW", mine, nullptr},
    {"THE ANCIENT TREE", ancientTree, leaves},
    {"A FALLEN STAR", fallenStar, starlight},
    {"THE COLOSSUS STIRS", colossus, bones},
    {"THE MONOLITH SPEAKS", monolith, runes},
    {"THE CRYSTAL SPIRE", spire, shimmer},
    {"THE GREAT MUSHROOM", mushroom, spores},
    {"A WRECK FULL OF PLUNDER", wreck, nullptr},
    {"THE ANCIENT GATE", gate, swirl},
    {"THE EMBER FORGE", forge, coals},
    {"THE FROZEN GIANT", frozen, frost},
};
