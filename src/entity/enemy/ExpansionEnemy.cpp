#include "entity/enemy/ExpansionEnemy.hpp"

#include "entity/Player.hpp"
#include "gfx/EnemyWarning.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {

typedef ExpansionEnemy X;

enum State { Idle, Winding, Dashing, Resting };

struct Def {
  Color tone;
  int immune;
  float range, windup, dash, speed, rest, warn;
};

constexpr Color VIOLET{0.9f, 0.6f, 0.9f, 1}, ICE{0.55f, 0.85f, 1, 1},
    MOSS{0.6f, 1, 0.4f, 1}, EMBER{1, 0.4f, 0.15f, 1};
constexpr int CHILL = int(StatusKind::Chill), POISON = int(StatusKind::Poison),
              BURN = int(StatusKind::Burn);

const Def DEFS[] = {
    {VIOLET, -1, 22, 0.35f, 1.8f, 260, 4.0f, 18},
    {VIOLET, -1, 70, 0.9f, 0.45f, 260, 3.0f, 52},
    {VIOLET, -1, 150, 0.65f, 0.45f, 260, 3.0f, 40},
    {VIOLET, -1, 150, 0.9f, 0.65f, 190, 3.0f, 40},
    {ICE, CHILL, 150, 0.9f, 0.45f, 260, 3.0f, 40},
    {ICE, CHILL, 150, 0.9f, 0.45f, 220, 4.5f, 40},
    {MOSS, POISON, 100, 0.9f, 0.45f, 260, 3.0f, 40},
    {MOSS, POISON, 150, 0.9f, 0.45f, 260, 3.0f, 40},
    {EMBER, BURN, 150, 0.9f, 0.45f, 260, 3.0f, 40},
    {EMBER, BURN, 150, 0.9f, 0.45f, 260, 3.0f, 40},
};

bool trails(int variant) {
  return variant == X::Wisp || variant == X::Sporeling || variant == X::Imp;
}

class EnemyHazard : public Entity {
public:
  int variant;
  float life, delay, pulse = 0, damage;
  const char *by;

  EnemyHazard(double x, double y, int variant, float radius, float life,
              float delay, float damage, const char *by)
      : variant(variant), life(life), delay(delay), damage(damage), by(by) {
    this->x = x;
    this->y = y;
    this->radius = radius;
  }

  void tick() override {
    if ((life -= DT) <= 0)
      return remove();
    if ((delay -= DT) > 0 || (pulse -= DT) > 0)
      return;
    pulse = 0.5f;
    Player &p = *level->player;
    if (p.dead())
      return;
    double xd = p.x - x, yd = p.y - y;
    float reach = radius + p.radius;
    if (xd * xd + yd * yd > reach * reach)
      return;
    p.felledBy = by;
    if (variant == X::Wisp)
      p.afflict(StatusKind::Chill, 0.7f, 0.3f);
    else if (variant == X::Sporeling)
      p.afflict(StatusKind::Poison, 1.5f, damage);
    else if (variant == X::Imp)
      p.afflict(StatusKind::Burn, 1.2f, damage);
    else {
      p.hurt(damage);
      life = std::min(life, 0.15f);
      Spark::burst(*level, x, y - 3, DEFS[variant].tone, 18, 90, true);
    }
  }

  void render(Screen &s) override {
    Color c = DEFS[variant].tone;
    float fade = std::min(1.0f, life);
    bool trail = trails(variant);
    if (delay > 0)
      enemyWarning(s, xp, yp, radius, trail ? 0.6f : 0.9f, life * 0.4f);
    else if (trail) {
      s.color(c.alpha(0.3f * fade));
      s.ring(xp, yp, radius);
    } else
      enemyWarning(s, xp, yp, radius, 0.55f * fade, life * 0.4f);
    if (delay <= 0) {
      s.color(c.alpha(0.2f * fade));
      s.center(atlas::Glow, xp, yp, radius * 2 / atlas::Glow.w);
    }
    s.color(col::white);
  }
};

} // namespace

ExpansionEnemy::ExpansionEnemy(const EnemyType &type, float hpMul,
                               float damageMul, int variant)
    : Enemy(type, hpMul, damageMul), variant(variant) {
  timer = 0.8f + variant * 0.13f;
  if (DEFS[variant].immune >= 0)
    status.immune |= 1u << DEFS[variant].immune;
}

void ExpansionEnemy::windup(float seconds, double xd, double yd,
                           double distance) {
  state = Winding;
  timer = seconds;
  aimX = distance > 1e-5 ? xd / distance : facing;
  aimY = distance > 1e-5 ? yd / distance : 0;
  xMark = x;
  yMark = y;
}

void ExpansionEnemy::burst(float reach, float damage, StatusKind kind,
                          float power) {
  Player &p = *level->player;
  double xd = p.x - x, yd = p.y - y;
  if (!p.dead() && xd * xd + yd * yd <= reach * reach) {
    p.felledBy = name();
    p.hurt(damage * damageMul);
    p.afflict(kind, 1.5f, power);
  }
  Spark::burst(*level, x, bodyY(), DEFS[variant].tone, 16, reach * 1.5f, true);
}

void ExpansionEnemy::hazard(float r, float life, float delay, float damage) {
  level->add<EnemyHazard>(x, y, variant, r, life, delay, damage * damageMul, name());
}

void ExpansionEnemy::tend() {
  timer = 2.0f;
  Enemy *nearby[24];
  for (int i = 0, n = level->gather(x, y, 70, nearby, 24); i < n; i++) {
    Enemy &e = *nearby[i];
    double dx = e.x - x, dy = e.y - y;
    if (e.removed || &e == this || dx * dx + dy * dy > 70 * 70)
      continue;
    e.status.effects[int(StatusKind::Burn)] = {};
    e.status.effects[int(StatusKind::Poison)] = {};
    e.hp = std::min(e.maxHp, e.hp + 3.0f);
  }
  flash = 0.15f;
  Spark::burst(*level, x, bodyY(), DEFS[variant].tone, 9, 50, true);
}

void ExpansionEnemy::stalk(Player &p, double xd, double yd, double dist) {
  const Def &d = DEFS[variant];
  if (variant == Vulture && dist > 45) {
    xa = -yd / dist * type.speed * 0.8;
    ya = xd / dist * type.speed * 0.8;
  }
  if (variant == Lantern && dist < 100 && dist > 48) {
    p.xa -= xd / dist * 35 * DT;
    p.ya -= yd / dist * 35 * DT;
  }
  if (timer <= 0 && dist < d.range)
    windup(d.windup, xd, yd, dist);
}

void ExpansionEnemy::release() {
  if (variant == Lantern || variant == Sporeling) {
    if (variant == Lantern)
      burst(52, 12, StatusKind::Marked, 0.2f);
    else
      hazard(40.0f, 3.0f, 0.25f, 3.5f);
    state = Resting;
    timer = variant == Lantern ? 4.5f : 5.0f;
    return;
  }
  state = Dashing;
  timer = DEFS[variant].dash;
  pulse = 0;
}

void ExpansionEnemy::charge(Player &p, double dist) {
  const Def &d = DEFS[variant];
  if (variant == Leech) {
    if (dist > 48) {
      state = Resting;
      timer = 3;
      return;
    }
    x = p.x - facing * 7;
    y = p.y + 2;
    xa = ya = 0;
    if (pulse <= 0) {
      pulse = 0.6f;
      float before = p.hp;
      p.felledBy = name();
      p.hurt(2.5f * damageMul, true);
      hp = std::min(maxHp, hp + std::max(0.0f, before - p.hp));
    }
  } else {
    xa = aimX * d.speed;
    ya = aimY * d.speed;
    if (variant == Iceback && dist < radius + p.radius + 6)
      p.afflict(StatusKind::Chill, 1.4f, 0.4f);
  }
  if (timer <= 0) {
    xa *= 0.2;
    ya *= 0.2;
    state = Resting;
    timer = d.rest;
  }
}

void ExpansionEnemy::tick() {
  Enemy::tick();
  if (removed)
    return;
  Player &p = *level->player;
  if (p.dead()) {
    state = Idle;
    return;
  }
  timer -= DT;
  pulse -= DT;
  double xd = p.x - x, yd = p.y - y;
  double dist = std::sqrt(xd * xd + yd * yd);

  switch (variant) {
  case Wisp:
  case Imp:
    if (pulse <= 0) {
      pulse = variant == Wisp ? 1.1f : 0.8f;
      hazard(18.0f, 2.4f, 0.35f, 3.0f);
    }
    return;
  case Brute:
    if (timer <= 0)
      tend();
    return;
  case Golem:
    if (timer <= 0 && dist < 70) {
      timer = 5;
      hazard(38.0f, 1.2f, 0.8f, 12.0f);
    }
    return;
  }

  if (state == Resting) {
    if (timer <= 0)
      state = Idle;
    return;
  }
  if (state == Idle)
    return stalk(p, xd, yd, dist);
  if (state == Winding) {
    x = xMark;
    y = yMark;
    xa = ya = 0;
    if (timer > 0)
      return;
    release();
    if (state != Dashing)
      return;
  }
  if (state == Dashing)
    charge(p, dist);
}

void ExpansionEnemy::render(Screen &s) {
  if (state == Winding) {
    Color c{1, 0.18f, 0.12f, 0.9f};
    enemyWarning(s, xp, yp, DEFS[variant].warn, 0.9f, timer);
    if (variant == Mantis || variant == Vulture || variant == Iceback)
      s.line(xp, yp, xp + aimX * 90, yp + aimY * 90, 2, c);
    s.color(col::white);
  }
  Enemy::render(s);
  if (variant == Brute) {
    s.color(DEFS[variant].tone.alpha(0.2f));
    s.ring(xp, yp, 70);
    s.color(col::white);
  }
}

void ExpansionEnemy::renderLight(Screen &s) {
  Enemy::renderLight(s);
  if (variant == Lantern || variant == Wisp || variant >= Imp)
    s.glow(xp, yp - 5, 42, DEFS[variant].tone * 0.12f);
}

void ExpansionEnemy::die() {
  if (variant == Golem)
    hazard(56.0f, 1.3f, 0.85f, 18.0f);
  Enemy::die();
}
