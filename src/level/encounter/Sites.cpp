#include "level/encounter/Common.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/SupplyBarrel.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/pickup/Magnet.hpp"
#include "entity/pickup/Potion.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/FlameDebris.hpp"

namespace encounter {

void cottage(Level *level, Site s) { level->add<Potion>(atX(s, 2), atY(s, 4)); }

void hearth(Level *level, Site s) {
  int lx, height;
  if (rng.chance(0.12f) && cottageChimney(s, &lx, &height))
    smoke(level, (s->x + lx) * 16 + 8.0, (s->y + 2) * 16 + 15.0, height + 4);
}

void graveyard(Level *level, Site s) {
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

void shrine(Level *, Site) {}

void camp(Level *level, Site s) {
  auto b = std::make_unique<SupplyBarrel>();
  b->x = atX(s, s->w / 2 + 1);
  b->y = atY(s, s->h / 2 + 1);
  level->add(std::move(b));
}

void campfire(Level *level, Site s) {
  int c = s->w / 2;
  double x = (s->x + c) * 16 + 8.0, y = (s->y + c + 1) * 16 + 1.0;
  for (int i = 0; i < 2; i++)
    FlameDebris::lick(*level, x + rng.range(-3, 3), y, 5, 1.2f);
  if (rng.chance(0.15f))
    smoke(level, x, y, 14);
}

void ruin(Level *level, Site s) {
  double x = atX(s, s->w / 2), y = atY(s, 2);
  if (s->seed & 1)
    level->add<Chest>(x, y);
  else
    level->add<Magnet>(x, y);
}

void castle(Level *level, Site s) {
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

void braziers(Level *level, Site s) {
  int gate = s->w / 2 - 1;
  double y = (s->y + s->h - 1) * 16 + 4.0;
  if (rng.chance(0.6f))
    FlameDebris::lick(*level, (s->x + gate) * 16 - 2.0, y, 6, 0.8f);
  if (rng.chance(0.6f))
    FlameDebris::lick(*level, (s->x + gate + 2) * 16 + 2.0, y, 6, 0.8f);
}

void stones(Level *level, Site s) {
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

void hum(Level *level, Site s) {
  if (Spark::alive > 500 || !rng.chance(0.25f))
    return;
  level->add<Spark>(s->centerX() + rng.range(-6, 6), s->centerY() - 6,
                    rng.range(-3, 3), rng.range(-24, -12),
                    Color{0.5f, 0.95f, 1, 1}, rng.range(0.6f, 1.2f), 0.3f,
                    true);
}

void mine(Level *level, Site s) {
  double x = (s->x + 3) * 16 + 8.0, y = (s->y + 2) * 16 + 4.0;
  level->add<Chest>((s->x + 3) * 16 + 8.0, (s->y + 5) * 16 + 8.0);
  for (int i = 0, n = 4 + int(level->time / 45); i < n; i++)
    level->spawner.spawn(i % 3 ? EnemyKind::Spider : EnemyKind::Bat,
                         x + rng.range(-10, 10), y + rng.range(-4, 4));
  Sound::horde.play(0.5f, 0.1f);
}

} // namespace encounter
