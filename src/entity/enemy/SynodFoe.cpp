#include "entity/enemy/SynodFoe.hpp"

#include <cmath>

#include "entity/Faction.hpp"
#include "entity/Player.hpp"
#include "gfx/Screen.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {

constexpr Color LANTERN{1, 0.78f, 0.42f, 1}, ASHEN{0.86f, 0.86f, 0.82f, 1}, GRACE{1, 0.86f, 0.5f, 1};

} // namespace

SynodFoe::SynodFoe(const EnemyType &type, float hpMul, float damageMul, int role)
    : Enemy(type, hpMul, damageMul), role(role), baseDamage(damageMul) {}

void SynodFoe::tick() {
  Enemy::tick();
  if (removed)
    return;
  if (role == Penitent)
    damageMul = baseDamage * (2 - hp / maxHp);
  if ((timer -= DT) > 0)
    return;
  timer = 1;
  if (role == Censer)
    smoke();
  if (role == Confessor || role == Prioress)
    preach();
  if (role == Limer || role == Prioress) {
    timer = LIME_EVERY;
    lime();
  }
}

void SynodFoe::lime() {
  if (!level->rules.matter)
    return;
  int tx = int(std::floor(x / 16)), ty = int(std::floor((y - 2) / 16));
  for (int dy = -1; dy <= 1; dy++)
    for (int dx = -1; dx <= 1; dx++)
      level->matter.set(tx + dx, ty + dy, Stuff::Limed, LIME_LIFE);
  Spark::burst(*level, x, y - 2, ASHEN, 4, 20, false);
}

void SynodFoe::preach() {
  level->enemiesNear(x, y, PREACH, [&](Enemy &e) {
    if (&e != this && factionOf(e) == Faction::Synod && e.hp < e.maxHp)
      e.hp = std::min(e.maxHp, e.hp + e.maxHp * 0.04f);
  });
  Spark::burst(*level, x, bodyY() - 6, GRACE, 2, 18, false);
}

void SynodFoe::smoke() {
  Player &p = *level->player;
  double xd = p.x - x, yd = p.y - y;
  if (xd * xd + yd * yd < SMOKE * SMOKE)
    p.afflict(StatusKind::Weaken, 1.5f, 1);
  Spark::burst(*level, x + facing * 6, y - 10, ASHEN, 2, 14, false);
}

void SynodFoe::renderLight(Screen &s) {
  Enemy::renderLight(s);
  if (role == Lamplighter || role == Prioress)
    s.glow(xp + facing * 6, yp - 14, 70, LANTERN * 0.4f);
}
