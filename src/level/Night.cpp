#include "level/Night.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/pickup/Nova.hpp"
#include "level/Level.hpp"
#include "particle/Explosion.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/Debris.hpp"
#include "particle/debris/FlameDebris.hpp"
#include "particle/debris/HealDebris.hpp"

namespace {
constexpr Color FLAME{0.55f, 0.34f, 0.12f, 1};

EnemyKind strongestUnlocked(float time) {
  for (int k = int(EnemyKind::Cyclops) - 1; k > 0; k--)
    if (Enemy::typeOf(EnemyKind(k)).unlockAt <= time)
      return EnemyKind(k);
  return EnemyKind::Bat;
}
} // namespace

float Night::left() const { return std::max(0.0f, dawnAt() - level.time); }

float Night::daylight() const {
  float d = clampf(1 - left() / 150, 0, 1);
  return d * d;
}

bool Night::isLit(const Structure &s) const {
  for (int i = 0; i < lit; i++)
    if (shrines[i].rx == s.rx && shrines[i].ry == s.ry)
      return true;
  return false;
}

bool Night::cold(const Structure *s, const void *night) {
  return s->kind == StructureKind::Shrine &&
         !static_cast<const Night *>(night)->isLit(*s);
}

void Night::tick() {
  Player &p = *level.player;
  int tx = int(std::floor(p.x / 16)), ty = int(std::floor(p.y / 16));
  Structure here;
  bool inside = world::structureAt(tx, ty, here) && cold(&here, this);
  if (inside)
    current = here;
  kindling =
      inside ? kindling + DT * level.kindleRate / KINDLE
             : std::max(0.0f, kindling - DT / 2);
  if (kindling >= 1)
    kindle();
  for (int i = 0; i < lit; i++) {
    double x = shrines[i].centerX(), y = shrines[i].centerY(), xd = x - p.x,
           yd = y - p.y;
    if (xd * xd + yd * yd > 380 * 380)
      continue;
    for (int k = 0; k < 3; k++)
      FlameDebris::lick(level, x + rng.range(-3, 3), y + 1, 9, 1.4f);
  }
  if ((scan -= DT) <= 0)
    scan = 0.5f,
    hasBeacon = world::nearestStructure(p.x, p.y, 7, beacon, cold, this);
  if (!dawn && level.time >= dawnAt())
    dawn = true, level.game.onDawn();
}

void Night::kindle() {
  offering = true;
  kindling = 0;
  if (lit < MAX_LIT)
    shrines[lit++] = current;
  level.game.codex.shrines++;
  level.litBiomes |= 1u << world::biomeAt(current.x + current.w / 2,
                                          current.y + current.h / 2);
  double x = current.centerX(), y = current.centerY();
  level.add<Nova>(x, y + 8);
  Spark::burst(level, x, y, col::gold, 20, 160, true);
  level.add<Explosion>(x, y, 6, 7);
  for (int i = 0; i < 14 && Debris::room(); i++)
    level.add<HealDebris>(x, y, 10, 1);
  int n = 8 + 3 * lit;
  EnemyKind kind = strongestUnlocked(level.time);
  for (int i = 0; i < n; i++) {
    double a = TAU * i / n;
    level.spawner.spawn(kind, x + std::cos(a) * 170, y + std::sin(a) * 170);
  }
  Sound::chest.play(0.8f, 0);
  level.game.announce(
      fmt::format("THE SHRINE BURNS  DAWN IN {}", clockText(left())));
}

void Night::renderLight(Screen &s) const {
  float t = level.time;
  for (int i = 0; i < lit; i++) {
    double x = shrines[i].centerX(), y = shrines[i].centerY();
    float f = flicker(t, 40.0f + i);
    s.glow(x, y - 8, 240 * (0.95f + 0.05f * f), FLAME * f);
    level.lights.add(x, y, 170, f);
  }
  if (kindling > 0)
    s.glow(current.centerX(), current.centerY() - 8, 60 + 180 * kindling,
           FLAME * kindling);
}
