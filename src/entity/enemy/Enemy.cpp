#include "entity/enemy/Enemy.hpp"
#include "entity/enemy/Boss.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/Ability.hpp"

#include "Game.hpp"
#include <spdlog/fmt/fmt.h>
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/AlphaWolf.hpp"
#include "entity/enemy/Burrower.hpp"
#include "entity/enemy/Cyclops.hpp"
#include "entity/enemy/Ghost.hpp"
#include "entity/enemy/Mimic.hpp"
#include "entity/enemy/Necromancer.hpp"
#include "entity/enemy/Renegade.hpp"
#include "entity/enemy/Thief.hpp"
#include "entity/enemy/ExpansionEnemy.hpp"
#include "entity/enemy/Wolf.hpp"
#include "entity/enemy/ai/Plans.hpp"
#include "entity/enemy/ai/Melee.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/pickup/Potion.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "entity/projectile/Orb.hpp"
#include "level/Level.hpp"
#include "particle/Arc.hpp"
#include "particle/TextParticle.hpp"
#include "particle/DeathEffect.hpp"
#include "particle/Drift.hpp"
#include "particle/Shockwave.hpp"
#include "particle/debris/Chip.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/Debris.hpp"
#include "particle/debris/FlameDebris.hpp"

Enemy::Enemy(const EnemyType &type, float hpMul, float damageMul)
    : type(type), damageMul(damageMul) {
  hp = maxHp = type.hp * hpMul;
  radius = type.radius;
  scale = type.scale;
  animT = rng.range(0, 10);
  seed = rng.next() | 1;
  side = seed & 2 ? 1 : -1;
}

float Enemy::roll() {
  seed ^= seed << 13, seed ^= seed >> 17, seed ^= seed << 5;
  return float(seed >> 8) / float(1 << 24);
}

void Enemy::voice(Sound &s, float volume, float jitter) const {
  if (level->game.screen.visible(x, y, 24))
    s.play(volume, jitter);
}

const Mob &Enemy::quarry() const {
  const Mob *best = level->player;
  double px = best->x - x, py = best->y - y, bd = px * px + py * py;
  for (const Ally *a : level->allies) {
    double xd = a->x - x, yd = a->y - y, d = xd * xd + yd * yd;
    float reach = a->def.lure ? 220.0f : 110.0f;
    if (a->removed || a->dead() || d > reach * reach)
      continue;
    if (a->def.lure)
      d *= 0.2;
    if (d < bd)
      best = a, bd = d;
  }
  return *best;
}

Sense Enemy::sense() const {
  const Mob &p = quarry();
  double rx = p.x - x, ry = p.y - y, raw = std::sqrt(rx * rx + ry * ry);
  double lead = std::min(0.7, raw / std::max(20.0, double(type.speed))) * 0.6;
  Sense s{rx + p.xa * lead, ry + p.ya * lead, 0, 0, 0, false};
  s.dist = std::sqrt(s.xd * s.xd + s.yd * s.yd);
  if (s.dist > 0.001)
    s.xn = s.xd / s.dist, s.yn = s.yd / s.dist;
  s.seen = s.dist < 300 && -(p.xAim * s.xn + p.yAim * s.yn) > 0.55;
  return s;
}

void Enemy::next(const Sense &s) {
  if (goals.empty())
    for (int i = 0; i < 6 && plan->steps[i]; i++)
      goals.push(plan->steps[i]);
  goalTime = 0;
  if (const Goal *g = goals.front())
    g->start(*this, s);
}

void Enemy::think() {
  remember();
  if (!plan)
    return void(xWant = yWant = 0);
  Sense s = sense();
  for (int i = 0; i < 2 && plan->interrupts[i]; i++) {
    const Goal *g = plan->interrupts[i];
    if (goals.front() != g && g->urgent(*this, s))
      goals.pushFront(g), goalTime = 0, g->start(*this, s);
  }
  if (goals.empty())
    next(s);
  double xs = 0, ys = 0;
  for (int step = 0; step < 4; step++) {
    const Goal *g = goals.front();
    if (!g || g->tick(*this, s, xs, ys) == GoalState::Running)
      break;
    goals.pop();
    next(s);
  }
  goalTime += DT, cooldown -= DT;
  xWant = xs, yWant = ys;
}

void Enemy::adopt() {
  wantsLeader = false;
  Enemy *nearby[24];
  for (int i = 0, n = level->gather(x, y, 140, nearby, 24); i < n; i++) {
    Enemy *o = nearby[i];
    if (o != this && &o->type == &type && o->leader == entt::null && o->aware) {
      leader = o->id;
      return;
    }
  }
}

void Enemy::land() {
  pounced = false, z = 0;
  level->add<Shockwave>(x, y, 3, 26, 0.3f, type.blood.alpha(0.8f), false);
  Player &p = *level->player;
  double xd = p.x - x, yd = p.y - y;
  if (!p.dead() && xd * xd + yd * yd < 28 * 28) {
    p.afflict(StatusKind::Chill, 0.5f, 0.85f);
    p.felledBy = name();
    p.hurt(type.damage * damageMul * 0.6f);
  }
}

void Enemy::act() {
  if (pounced)
    land();
  if (wantsLeader)
    adopt();
  if (!wantsFire)
    return;
  wantsFire = false;
  Player &p = *level->player;
  double xd = p.x - x, yd = p.bodyY() - bodyY(),
         dd = std::sqrt(xd * xd + yd * yd), lead = dd / 70 * 0.7;
  xd += p.xa * lead, yd += p.ya * lead;
  dd = std::sqrt(xd * xd + yd * yd);
  if (p.dead() || dd < 1e-5)
    return;
  level->add<Orb>(x, bodyY(), xd / dd * 70, yd / dd * 70, 10 * damageMul * status.outgoing())
      ->by = name();
  if (traits.has(Trait::Multishot))
    for (int k = -1; k <= 1; k += 2) {
      double a = std::atan2(yd, xd) + k * 0.3;
      level->add<Orb>(x, bodyY(), std::cos(a) * 70, std::sin(a) * 70,
                      10 * damageMul * status.outgoing())->by = name();
    }
  if (boss) {
    double turn = std::atan2(yd, xd) + PI / 10;
    for (int i = 0; i < 10; i++) {
      double a = turn + TAU * i / 10;
      level->add<Orb>(x, bodyY(), std::cos(a) * 45, std::sin(a) * 45,
                      8 * damageMul * status.outgoing())->by = name();
    }
    level->add<Shockwave>(x, bodyY(), 6, 40, 0.4f, Color{0.75f, 0.35f, 1, 0.9f})
        ->spokes = 10;
  }
  Sound::orb.play(0.3f);
}

void Enemy::onAdded() { level->tag(this), level->arrive(this); }

void Enemy::move() {
  if (!plan)
    return;
  const Player &p = *level->player;
  float spdMul = 1.0f;
  if (frenzy > 0) {
    frenzy = std::max(0.0f, frenzy - DT);
    spdMul = 1.35f;
  }
  spdMul *= status.movement();
  if (status.afraid() && !boss) {
    double fx = x - p.x, fy = y - p.y, fd = std::max(1.0, std::sqrt(fx * fx + fy * fy));
    xWant = fx / fd * type.speed, yWant = fy / fd * type.speed;
  }
  if (level->rules.route)
    level->rules.route(x, y, p.x, p.y, type.speed, xWant, yWant);
  spdMul *= traits.speed();
  if (!type.floats && level->surfaceAt(x, y) == 1)
    spdMul *= is(EnemyKind::MireLeech) ? 1.2f : 0.5f;
  if (!type.floats)
    spdMul *= level->climb(x, y, xWant, yWant);
  x += (xWant * level->pace * spdMul + xa) * DT;
  y += (yWant * level->pace * spdMul + ya) * DT;
  static const double KNOCK_DECAY = std::exp(-9 * DT);
  xa *= KNOCK_DECAY, ya *= KNOCK_DECAY;
  flash = std::max(0.0f, flash - DT);
  knifeCooldown -= DT;
  animT += DT;
  if (type.floats)
    z = 4 - std::sin(animT * 7) * 2;
  double xd = p.x - x, yd = p.y - y;
  if (std::abs(xWant) > 4)
    facing = xWant < 0 ? -1.0f : 1.0f;
  else if (std::abs(xd) > 2)
    facing = xd < 0 ? -1.0f : 1.0f;
  if (xd * xd + yd * yd > 560 * 560) {
    bool running = p.xa * p.xa + p.ya * p.ya > 100;
    double dir = (running ? std::atan2(p.ya, p.xa) : roll() * TAU) +
                 roll() * 2 - 1,
           r = 290 + 30 * roll();
    x = p.x + std::cos(dir) * r;
    y = p.y + std::sin(dir) * r;
  }
}

void Enemy::push() {
  xPush = yPush = 0;
  if (!plan || type.floats)
    return;
  double xd = level->player->x - x, yd = level->player->y - y;
  if (xd * xd + yd * yd > 260 * 260 &&
      (entt::to_integral(id) + level->ticks) & 1)
    return;
  Enemy *nearby[24];
  for (int i = 0, count = level->gather(x, y, radius + 16, nearby, 24);
       i < count; i++) {
    const Enemy &o = *nearby[i];
    if (&o == this || o.type.floats)
      continue;
    double ox = x - o.x, oy = y - o.y;
    double min = (radius + o.radius) * 0.85, d2 = ox * ox + oy * oy;
    if (d2 >= min * min || d2 < 1e-6)
      continue;
    double dd = std::sqrt(d2),
           share = o.type.speed <= 0 ? 1.0 : o.radius / (radius + o.radius);
    double k = (min - dd) / dd * share;
    xPush += ox * k, yPush += oy * k;
  }
}

void Enemy::settle() {
  if (!plan)
    return;
  x += xPush, y += yPush;
  if (!type.floats)
    level->collide(x, y, radius);
  if (level->rules.wall && !is(EnemyKind::Shade))
    level->rules.wall(xOld, yOld, x, y);
  xOld = x, yOld = y;
}

void Enemy::tick() {
  status.moving = xWant * xWant + yWant * yWant > 25;
  status.tick(DT, [&](float damage, Upgrade *src) {
    hurt(damage, 0, 0, 0, src, true);
    return !removed;
  });
  if (removed)
    return;
  act();
  if (!type.floats)
    traits.tick(*this);
  if (!upgrades.empty()) {
    aim(xAim, yAim);
    tickKit();
  }
  if (def && phase < 2 && hp < maxHp * float(2 - phase) / 3)
    enrage();
  if (castCount)
    channel();
  if (removed)
    return;
  Player &p = *level->player;
  double xd = p.x - x, yd = p.y - y, dist = std::sqrt(xd * xd + yd * yd);
  float reach = type.radius + 5;
  if (dist < reach && dist > 1e-3 && !p.dead()) {
    x = p.x - xd / dist * reach;
    y = p.y - yd / dist * reach;
    p.felledBy = name();
    bool lands = p.invuln <= 0 && !p.god;
    float before = p.hp;
    p.hurt(type.damage * damageMul * status.outgoing() * contactBonus(p));
    if (lands)
      traits.landed(*this, std::max(before - p.hp, maxHp * 0.1f)), bit(p);
    if (lands && p.bonus().thorns > 0)
      hurt(p.bonus().thorns, x - p.x, y - p.y, 20, nullptr, true);
  }
}

void Enemy::render(Screen &s) {
  if (title && !def) {
    float beat = 0.55f + 0.3f * std::sin(animT * 4);
    s.ring(xp, yp, radius + 7, col::gold.alpha(beat));
    s.ring(xp, yp - 18 * scale, 3, col::gold.alpha(0.9f));
  }
  if (boss) {
    float beat = 0.5f + 0.25f * std::sin(age * 5);
    s.ring(xp, yp, radius + 6, Color{0.9f, 0.15f, 0.2f, beat});
    s.ring(xp, yp, radius + 10, Color{0.9f, 0.15f, 0.2f, beat * 0.4f});
  }
  if (wind > 0 && winding) {
    float k = 1 - wind / (boss ? WIND * 1.4f : WIND), r = radius + 8;
    s.ring(xp, yp - 6 * scale, r + 8 - 8 * k, winding->tone.alpha(0.35f + 0.6f * k));
    s.ring(xp, yp - 6 * scale, r + 14 - 12 * k, winding->tone.alpha(0.25f * k));
  }
  Color c = tint();
  traits.render(s, *this, radius, age);
  if (traits.any()) {
    Color a = traits.tone();
    c = {c.r * (0.7f + 0.3f * a.r), c.g * (0.7f + 0.3f * a.g),
         c.b * (0.7f + 0.3f * a.b), c.a * (traits.veiled() ? 0.35f : 1)};
  }
  if (const Goal *g = goals.front())
    if (float t = g->tell(*this); t > 0) {
      c = {c.r, c.g * (1 - 0.6f * t), c.b * (1 - 0.6f * t), c.a};
      if (dynamic_cast<const Melee *>(g)) {
        double ex = xp + xMark * (radius + 13), ey = yp + yMark * (radius + 13);
        Color warning{1, 0.18f, 0.12f, 0.55f + 0.45f * t};
        s.line(xp + xMark * radius, yp + yMark * radius, ex, ey, 2, warning);
        s.line(ex, ey, ex - xMark * 5 - yMark * 3, ey - yMark * 5 + xMark * 3, 2, warning);
        s.line(ex, ey, ex - xMark * 5 + yMark * 3, ey - yMark * 5 - xMark * 3, 2, warning);
      }
    }
  drawBody(s, xp, yp, sprite(), type.floats, 1, c, flash > 0 ? 1.0f : 0.0f,
           col::white);
}

void Enemy::renderLight(Screen &s) {
  if (boss)
    s.glow(xp, yp - 7 * scale, 130, {0.35f, 0.06f, 0.08f, 1});
  if (elite)
    s.glow(xp, yp - 7 * scale, 40 * scale, traits.tone() * 0.12f);
  if (title && !def)
    s.glow(xp, yp - 7 * scale, 70, Color{0.35f, 0.26f, 0.06f, 1});
  if (wind > 0 && winding)
    s.glow(xp, yp - 6 * scale, 50 * scale, winding->tone * 0.25f);
}

void Enemy::renderShadow(Screen &s) { drawShadow(s, xp, yp, type.floats); }

Sprite Enemy::sprite() const {
  if (type.art == atlas::beast && !type.floats && status.moving && std::sin(animT * 7) > 0)
    return atlas::beast(type.tile + atlas::bs::Step);
  return type.art ? type.art(type.tile) : atlas::tile(type.tile);
}

void Enemy::temper(StatusKind kind, float &duration, float &power) const {
  if (!level)
    return;
  const ItemStats &b = level->player->bonus();
  duration *= 1 + b.weave / 100;
  if (kind == StatusKind::Burn)
    power *= 1 + b.ember / 100;
  if ((kind == StatusKind::Burn || kind == StatusKind::Poison) && b.catalyst > 0 &&
      status.power(StatusKind::Marked) > 0)
    power *= 1 + b.catalyst / 100;
}

void Enemy::onHit(float damage, Upgrade *src) {
  Player &p = *level->player;
  const ItemStats &b = p.bonus();
  if (b.burn > 0 && rng.chance(b.burn / 100))
    afflict(StatusKind::Burn, 2, damage * 0.2f, src);
  if (b.chill > 0 && rng.chance(b.chill / 100))
    afflict(StatusKind::Chill, 1.2f, 0.3f, src);
  if (b.poison > 0 && rng.chance(b.poison / 100))
    afflict(StatusKind::Poison, 3, damage * 0.15f, src);
  if (b.bleed > 0 && rng.chance(b.bleed / 100))
    afflict(StatusKind::Bleed, 3, damage * 0.12f, src);
  if (b.leech > 0)
    p.mend(std::min(damage * b.leech / 100, 0.5f));
  if (b.shock > 0 && rng.chance(b.shock / 100))
    for (Enemy *o : level->nearest(x, y, 80, 3))
      if (o != this && !o->removed) {
        level->add<Arc>(std::vector<ArcNode>{{x, bodyY()}, {o->x, o->bodyY()}}, 0.2f,
                        Color{0.6f, 0.8f, 1, 1});
        o->hurt(damage * 0.5f * p.arcs, o->x - x, o->y - y, 0, nullptr, true);
        break;
      }
}

void Enemy::hurt(float damage, double xd, double yd, float force,
                 Upgrade *src, bool periodic) {
  if (removed)
    return;
  if (src)
    garrison = false;
  double dd = std::sqrt(xd * xd + yd * yd);
  if (dd > 1e-5)
    xd /= dd, yd /= dd;
  else
    xd = yd = 0;
  if (!periodic)
    damage *= status.incoming();
  if (!periodic)
    damage = traits.incoming(*this, damage) * shell(xd, yd);
  if (src && !type.floats &&
      level->heightAt(level->player->x, level->player->y) > level->heightAt(x, y))
    damage *= 1.15f;
  Player &p = *level->player;
  bool ours = !(src && src->hostile());
  if (ours && !periodic)
    level->xNoise = x, level->yNoise = y, level->noiseAt = level->time;
  bool crit = !periodic && rng.chance(0.08f + p.luck);
  if (!periodic && !crit && p.sure > 0)
    crit = true, p.sure--;
  if (crit)
    damage *= (p.hero ? p.hero->edge() : 1.8f) * (1 + p.bonus().critDamage / 100), p.crits++;
  if (ours && p.bonus().coldBlood > 0 && status.power(StatusKind::Chill) > 0)
    damage *= 1 + p.bonus().coldBlood / 100;
  if (ours && p.bonus().prophecy > 0 && status.power(StatusKind::Marked) > 0)
    damage *= 1 + p.bonus().prophecy / 100;
  if (ours)
    force *= 1 + p.bonus().juggernaut / 100;
  if (src && !src->hostile())
    damage *= 1 + ((boss || def ? level->player->bonus().bossDamage : 0) +
                   (elite || title ? level->player->bonus().eliteDamage : 0) +
                   (level->night.dawn ? 0 : level->player->bonus().nightDamage)) /
                      100;
  if (src) {
    src->dealt += std::min(damage, std::max(hp, 0.0f));
    if (!periodic)
      level->player->traits.landed(*level->player, damage * 0.1f);
  }
  level->totalDamage += damage;
  hp -= damage;
  flash = 0.1f;
  aware = true;
  if (!alarmed) {
    alarmed = true;
    level->enemiesNear(x, y, 90, [](Enemy &o) { o.aware = true; });
  }
  float f = force * type.knockResist * (boss ? 0.15f : elite ? 0.5f : 1.0f);
  xa += xd * f;
  ya += yd * f;
  if (src && !src->hostile() && !periodic)
    onHit(damage, src);
  if (ours && !periodic && p.bonus().staticBuild > 0 &&
      ++p.hits >= 12 - int(p.bonus().staticBuild)) {
    p.hits = 0;
    ability::chain(p, 3, 200, 10);
  }
  TextParticle::pop(*level, x, bodyY(), int(std::lround(damage)),
                    crit ? col::gold : col::white);
  Sound::hit.play(0.35f);
  if (hp <= 0)
    die();
}

