#include "entity/enemy/Renegade.hpp"

#include "entity/Ability.hpp"
#include "entity/pickup/Potion.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

const Arms Renegade::ARMS[COUNT] = {
    {"Hunter's Bow",
     {[](Enemy &e) { ability::keg(e, 36); }, 8, 200, {1, 0.62f, 0.25f, 1}}},
    {"Hurled Axe",
     {[](Enemy &e) { ability::charge(e, 100, 30); }, 7, 150, {0.95f, 0.3f, 0.25f, 1}}},
    {"Orbiting Knives",
     {[](Enemy &e) { ability::lunge(e, 80, 26); }, 6, 110, {1, 0.85f, 0.5f, 1}}},
    {"Warding Halo",
     {[](Enemy &e) { ability::rally(e, 60, 20, 2); }, 9, 90, {1, 0.82f, 0.3f, 1}}},
};

Renegade::Renegade(const EnemyType &type, float hpMul, float damageMul,
                   int index)
    : Enemy(type, hpMul, damageMul), arms(ARMS[index]) {
  casts = &arms.cast, castCount = 1;
  castTime = arms.cast.every * 0.5f;
}

void Renegade::onAdded() {
  Enemy::onAdded();
  might = 0.55f, haste = 1.5f;
  grant(arms.weapon, 1);
}

void Renegade::die() {
  Enemy::die();
  Spark::burst(*level, x, bodyY(), arms.cast.tone, 10, 80, true);
  if (rng.chance(0.35f))
    level->add<Potion>(x + 6, y);
}
