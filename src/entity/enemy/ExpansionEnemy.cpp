#include "entity/enemy/ExpansionEnemy.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {
Color tone(int kind) {
  if (kind == 4 || kind == 5)
    return {0.55f, 0.85f, 1, 1};
  if (kind == 6 || kind == 7)
    return {0.6f, 1, 0.4f, 1};
  if (kind >= 8)
    return {1, 0.4f, 0.15f, 1};
  return {0.9f, 0.6f, 0.9f, 1};
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
    if (variant == 4)
      p.afflict(StatusKind::Chill, 0.7f, 0.3f);
    else if (variant == 6)
      p.afflict(StatusKind::Poison, 1.5f, damage);
    else if (variant == 8)
      p.afflict(StatusKind::Burn, 1.2f, damage);
    else {
      p.hurt(damage);
      life = std::min(life, 0.15f);
      Spark::burst(*level, x, y - 3, tone(variant), 18, 90, true);
    }
  }

  void render(Screen &s) override {
    Color c = tone(variant);
    s.color(c.alpha(delay > 0 ? 0.65f : 0.3f * std::min(1.0f, life)));
    s.center(atlas::Ring, xp, yp, radius * 2 / atlas::Ring.w, life * 0.4f);
    if (delay <= 0)
      s.center(atlas::Glow, xp, yp, radius * 2 / atlas::Glow.w);
    s.color(col::white);
  }
};
}

ExpansionEnemy::ExpansionEnemy(const EnemyType &type, float hpMul,
                               float damageMul, int variant)
    : Enemy(type, hpMul, damageMul), variant(variant) {
  timer = 0.8f + variant * 0.13f;
  if (variant == 4 || variant == 5)
    status.immune |= 1u << int(StatusKind::Chill);
  if (variant == 6 || variant == 7)
    status.immune |= 1u << int(StatusKind::Poison);
  if (variant >= 8)
    status.immune |= 1u << int(StatusKind::Burn);
}

void ExpansionEnemy::windup(float seconds, double xd, double yd,
                           double distance) {
  state = 1;
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
  Spark::burst(*level, x, bodyY(), tone(variant), 16, reach * 1.5f, true);
}

void ExpansionEnemy::tick() {
  Enemy::tick();
  if (removed)
    return;
  Player &p = *level->player;
  if (p.dead()) {
    state = 0;
    return;
  }
  timer -= DT;
  pulse -= DT;
  double xd = p.x - x, yd = p.y - y;
  double dist = std::sqrt(xd * xd + yd * yd);

  if (variant == 4 || variant == 8) {
    if (pulse <= 0) {
      pulse = variant == 4 ? 1.1f : 0.8f;
      level->add<EnemyHazard>(x, y, variant, 18.0f, 2.4f, 0.35f,
                              3.0f * damageMul, name());
    }
    return;
  }

  if (variant == 7) {
    if (timer <= 0) {
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
      Spark::burst(*level, x, bodyY(), tone(variant), 9, 50, true);
    }
    return;
  }

  if (variant == 9) {
    if (timer <= 0 && dist < 70) {
      timer = 5;
      level->add<EnemyHazard>(x, y, variant, 38.0f, 1.2f, 0.8f,
                              12.0f * damageMul, name());
    }
    return;
  }

  if (state == 3) {
    if (timer <= 0)
      state = 0;
    return;
  }

  if (state == 0) {
    if (variant == 3 && dist > 45) {
      xa = -yd / dist * type.speed * 0.8;
      ya = xd / dist * type.speed * 0.8;
    }
    if (variant == 1 && dist < 100 && dist > 48) {
      p.xa -= xd / dist * 35 * DT;
      p.ya -= yd / dist * 35 * DT;
    }
    float range = variant == 0 ? 22.0f : variant == 1 ? 70.0f :
                  variant == 6 ? 100.0f : 150.0f;
    if (timer <= 0 && dist < range)
      windup(variant == 0 ? 0.35f : variant == 2 ? 0.65f : 0.9f,
             xd, yd, dist);
    return;
  }

  if (state == 1) {
    x = xMark;
    y = yMark;
    xa = ya = 0;
    if (timer > 0)
      return;
    if (variant == 1 || variant == 6) {
      if (variant == 1)
        burst(52, 12, StatusKind::Marked, 0.2f);
      else
        level->add<EnemyHazard>(x, y, variant, 40.0f, 3.0f, 0.25f,
                                3.5f * damageMul, name());
      state = 3;
      timer = variant == 1 ? 4.5f : 5.0f;
      return;
    }
    state = 2;
    timer = variant == 0 ? 1.8f : variant == 3 ? 0.65f : 0.45f;
    pulse = 0;
  }

  if (state == 2) {
    if (variant == 0) {
      if (dist > 48) {
        state = 3;
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
      float speed = variant == 5 ? 220.0f : variant == 3 ? 190.0f : 260.0f;
      xa = aimX * speed;
      ya = aimY * speed;
      if (variant == 5 && dist < radius + p.radius + 6)
        p.afflict(StatusKind::Chill, 1.4f, 0.4f);
    }
    if (timer <= 0) {
      xa *= 0.2;
      ya *= 0.2;
      state = 3;
      timer = variant == 0 ? 4.0f : variant == 5 ? 4.5f : 3.0f;
    }
  }
}

void ExpansionEnemy::render(Screen &s) {
  if (state == 1) {
    Color c = tone(variant).alpha(0.8f);
    float reach = variant == 0 ? 18.0f : variant == 1 ? 52.0f : 40.0f;
    s.color(c);
    s.center(atlas::Ring, xp, yp, reach * 2 / atlas::Ring.w, timer);
    if (variant == 2 || variant == 3 || variant == 5)
      s.line(xp, yp, xp + aimX * 90, yp + aimY * 90, 2, c);
    s.color(col::white);
  }
  Enemy::render(s);
  if (variant == 7) {
    s.color(tone(variant).alpha(0.2f));
    s.center(atlas::Ring, xp, yp, 140.0f / atlas::Ring.w, animT * 0.3f);
    s.color(col::white);
  }
}

void ExpansionEnemy::renderLight(Screen &s) {
  Enemy::renderLight(s);
  if (variant == 1 || variant == 4 || variant >= 8)
    s.glow(xp, yp - 5, 42, tone(variant) * 0.12f);
}

void ExpansionEnemy::die() {
  if (variant == 9)
    level->add<EnemyHazard>(x, y, variant, 56.0f, 1.3f, 0.85f,
                            18.0f * damageMul, name());
  Enemy::die();
}
