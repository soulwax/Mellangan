#include "entity/enemy/WildFoe.hpp"

#include "entity/Ability.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/enemy/ai/Mend.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "particle/Arc.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace {

constexpr Color HEX{0.55f, 0.85f, 0.35f, 1}, TONGUE{0.9f, 0.45f, 0.5f, 1},
                WATER{0.45f, 0.7f, 0.9f, 1}, VINE{0.4f, 0.7f, 0.3f, 1},
                SPORE{0.75f, 0.6f, 0.85f, 1};

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

void tongue(Enemy &e) {
  Player &p = *e.level->player;
  double xd = e.x - p.x, yd = e.y - p.y, d = std::sqrt(xd * xd + yd * yd);
  if (p.dead() || d > 95 || d < 1)
    return;
  e.level->add<Arc>(std::vector<ArcNode>{{e.x, e.bodyY()}, {p.x, p.bodyY()}}, 0.25f, TONGUE);
  double pull = std::min(30.0, d - 14);
  p.x += xd / d * pull, p.y += yd / d * pull;
  e.level->collide(p.x, p.y, p.radius);
  p.felledBy = e.name();
  p.hurt(4 * e.damageMul);
}

void drown(Enemy &e) {
  for (int i = 0; i < 2; i++) {
    float a = e.roll() * TAU;
    e.level->spawner.spawn(EnemyKind::Drowned, e.x + std::cos(a) * 60, e.y + std::sin(a) * 60);
  }
  e.level->add<Shockwave>(e.x, e.bodyY(), 4, 40, 0.5f, WATER, false);
}

bool wet(double x, double y) {
  return world::isWater(int(std::floor(x / 16)), int(std::floor(y / 16)));
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
    {{tongue, 5, 90, TONGUE}, 1, 0},
    {{}, 0, 0, false, false, false, false, true, false, true},
    {{}, 0, 0, false, false, false, false, false, false, true, true},
    {{}, 0, 0, false, false, false, false, false, false, false, false, true},
    {{}, 0, 0, false, false, false, false, false, true},
    {{}, 0, 0, false, false, false, false, false, false, false, false, false, true},
    {{drown, 8, 220, WATER}, 1, 0},
};
static_assert(std::size(WildFoe::DEFS) == int(WildFoe::LAST) - int(WildFoe::FIRST) + 1);

WildFoe::WildFoe(const EnemyType &type, float hpMul, float damageMul, int index)
    : Enemy(type, hpMul, damageMul), wild(DEFS[index]) {
  if (wild.casts)
    casts = &wild.cast, castCount = 1, castTime = wild.cast.every * 0.5f;
}

void WildFoe::onAdded() {
  Enemy::onAdded();
  surface();
}

void WildFoe::surface() {
  xHome = x, yHome = y;
  if (!wild.waterborn)
    return;
  for (int i = 0; i < 16 && !wet(x, y); i++) {
    float a = roll() * TAU, r = 16 + roll() * 96;
    if (wet(xHome + std::cos(a) * r, yHome + std::sin(a) * r))
      x = xHome + std::cos(a) * r, y = yHome + std::sin(a) * r;
  }
  if (!wet(x, y))
    return remove();
  xHome = x, yHome = y;
  buried = wild.lurks;
  level->add<Shockwave>(x, y, 2, 14, 0.4f, WATER, false);
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
  if (p.dead())
    return;
  double xd = p.x - x, yd = p.y - y, d2 = xd * xd + yd * yd;
  float reach = type.radius + p.radius + 4;
  if (wild.dims && d2 < reach * reach)
    p.dim = 4;
  if (wild.roots)
    strangle(p);
  if (wild.puffs && d2 < (reach + 4) * (reach + 4))
    puff(p);
}

void WildFoe::strangle(Player &p) {
  x = xHome, y = yHome;
  if ((rootTime -= DT) > 0)
    return;
  rootTime = 2.2f;
  double xd = x - p.x, yd = y - p.y, d = std::sqrt(xd * xd + yd * yd);
  if (d > 70 || d < 1)
    return;
  p.afflict(StatusKind::Chill, 0.9f, 0.75f);
  double pull = std::min(12.0, d - 16);
  p.x += xd / d * pull, p.y += yd / d * pull;
  level->collide(p.x, p.y, p.radius);
  level->add<Arc>(std::vector<ArcNode>{{x, bodyY()}, {p.x, p.bodyY()}}, 0.35f, VINE);
}

void WildFoe::puff(Player &) {
  ability::miasma(*this, 55, 4 * damageMul, 3);
  Spark::burst(*level, x, bodyY(), SPORE, 8, 60, true);
  hurt(maxHp * 10, 0, 0, 0);
}

void WildFoe::render(Screen &s) {
  if (!buried)
    return Enemy::render(s);
  float k = 0.5f + 0.5f * std::sin(animT * 3);
  s.ring(xp, yp - 2, 4 + 2 * k, WATER.alpha(0.35f));
  s.ring(xp + 5, yp, 2 + k, WATER.alpha(0.25f));
}

void WildFoe::renderShadow(Screen &s) {
  if (!buried)
    Enemy::renderShadow(s);
}

float WildFoe::contactBonus(const Player &p) const {
  return wild.backstab && (x - p.x) * p.facing < 0 ? 1.5f : 1.0f;
}

float WildFoe::shell(double xd, double yd) const {
  if (!wild.guarded || (xd == 0 && yd == 0))
    return 1;
  double facing = xd * xMark + yd * yMark;
  return facing < -0.3 ? 0.3f : facing > 0.3 ? 1.3f : 1.0f;
}

void WildFoe::bit(Player &p) {
  if (wild.chills)
    p.afflict(StatusKind::Chill, 1.5f, 0.35f);
}

void WildFoe::die() {
  if (wild.bramble)
    level->add<Bramble>(x, y);
  Enemy::die();
}
