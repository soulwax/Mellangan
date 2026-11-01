#include "entity/enemy/WildFoe.hpp"

#include "entity/Ability.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/enemy/ai/Mend.hpp"
#include "level/Level.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace {

constexpr Color HEX{0.55f, 0.85f, 0.35f, 1};

void volley(Enemy &e) { ability::volley(e, 3, 0.22, 150, 6, 0); }

void hex(Enemy &e) {
  Player &p = *e.level->player;
  double xd = p.x - e.x, yd = p.y - e.y;
  if (xd * xd + yd * yd > 170 * 170)
    return;
  p.afflict(StatusKind::Weaken, 3, 0.25f);
  e.level->add<Shockwave>(p.x, p.bodyY(), 3, 22, 0.35f, HEX);
  Spark::burst(*e.level, p.x, p.bodyY(), HEX, 6, 40, true);
}

} // namespace

const WildDef WildFoe::DEFS[] = {
    {{}, 0, 0, false, false, true, false},
    {{}, 0, 0, false, false, false, false},
    {{volley, 3.2f, 210, {0.95f, 0.8f, 0.5f, 1}}, 1, 0, false, false, false, false},
    {{hex, 6.5f, 170, HEX}, 1, 0, false, false, false, true},
    {{}, 0, 25, false, false, false, false},
    {{}, 0, 0, false, false, false, false},
    {{}, 0, 0, true, false, false, false},
    {{}, 0, 0, false, true, false, false},
};

WildFoe::WildFoe(const EnemyType &type, float hpMul, float damageMul, int index)
    : Enemy(type, hpMul, damageMul), wild(DEFS[index]) {
  if (wild.casts)
    casts = &wild.cast, castCount = 1, castTime = wild.cast.every * 0.5f;
}

void WildFoe::tick() {
  Enemy::tick();
  if (removed)
    return;
  life += DT;
  Player &p = *level->player;
  if (wild.grow > 0 && life >= wild.grow) {
    level->spawner.spawn(EnemyKind::Treant, x, y);
    level->add<Shockwave>(x, bodyY(), 4, 30, 0.4f, Color{0.45f, 0.8f, 0.35f, 1}, false);
    return remove();
  }
  if (wild.mends && (mendTime -= DT) <= 0) {
    mendTime = 1;
    if (Enemy *ally = hurtAlly(*this, 90)) {
      ally->hp = std::min(ally->maxHp, ally->hp + 4 * damageMul);
      Spark::burst(*level, ally->x, ally->bodyY(), HEX, 4, 30, true);
    }
  }
  if (wild.dims && !p.dead()) {
    double xd = p.x - x, yd = p.y - y;
    float reach = type.radius + p.radius + 4;
    if (xd * xd + yd * yd < reach * reach)
      p.dim = 4;
  }
}

float WildFoe::contactBonus(const Player &p) const {
  return wild.backstab && (x - p.x) * p.facing < 0 ? 1.5f : 1.0f;
}

void WildFoe::die() {
  if (wild.bramble)
    level->add<Bramble>(x, y);
  Enemy::die();
}
