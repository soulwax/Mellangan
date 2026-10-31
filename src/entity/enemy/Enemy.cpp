#include "entity/enemy/Enemy.hpp"
#include "entity/enemy/Hazard.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/AlphaWolf.hpp"
#include "entity/enemy/Burrower.hpp"
#include "entity/enemy/Cyclops.hpp"
#include "entity/enemy/Ghost.hpp"
#include "entity/enemy/Necromancer.hpp"
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
#include "particle/TextParticle.hpp"
#include "particle/DeathEffect.hpp"
#include "particle/Drift.hpp"
#include "particle/Shockwave.hpp"
#include "particle/debris/Chip.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/Debris.hpp"
#include "particle/debris/FlameDebris.hpp"


namespace {
namespace t = atlas::t;
namespace fr = atlas::fr;

const EnemyType TYPES[] = {
    {t::Bat, 5, 50, 5, 5, 1, 1, 1, {0.85f, 0.55f, 0.35f, 1}, true, 0},
    {t::Rat, 9, 40, 6, 6, 1, 1, 1, {0.75f, 0.55f, 0.4f, 1}, false, 25},
    {t::Slime, 16, 30, 8, 6, 1, 2, 1, {0.3f, 0.95f, 0.6f, 1}, false, 50},
    {t::Scorpion, 14, 58, 7, 6, 1, 2, 1, {0.7f, 0.75f, 0.8f, 1}, false, 70},
    {t::Spider, 22, 48, 10, 6, 1, 3, 1, {0.9f, 0.6f, 0.4f, 1}, false, 100},
    {t::Ghost, 30, 36, 12, 6, 1, 4, 1, {0.8f, 0.85f, 1, 1}, true, 160},
    {t::Cultist, 40, 38, 12, 6, 1, 5, 1, {0.6f, 0.3f, 0.8f, 1}, false, 200},
    {t::Demon, 70, 32, 16, 8, 1.25f, 8, 0.8f, {1, 0.3f, 0.25f, 1}, false, 240},
    {t::Knight,
     120,
     26,
     18,
     7,
     1.15f,
     10,
     0.4f,
     {0.75f, 0.78f, 0.85f, 1},
     false,
     300},
    {t::Cyclops,
     900,
     30,
     28,
     15,
     2.5f,
     40,
     0.15f,
     {1, 0.75f, 0.5f, 1},
     false,
     1e9f},
    {fr::Wolf, 28, 62, 9, 7, 1.0f, 3, 0.85f, {0.6f, 0.15f, 0.15f, 1}, false, 40,
     atlas::frontier},
    {fr::Alpha, 85, 54, 16, 9, 1.3f, 8, 0.5f, {0.55f, 0.12f, 0.12f, 1}, false, 80,
     atlas::frontier},
    {fr::Burrower, 48, 65, 15, 8, 1.1f, 5, 0.6f, {0.85f, 0.55f, 0.3f, 1}, false, 60,
     atlas::frontier},
    {fr::Necromancer, 60, 30, 10, 7, 1.1f, 7, 0.7f, {0.35f, 0.1f, 0.45f, 1}, false, 110,
     atlas::frontier},
    {fr::Skeleton, 14, 42, 7, 6, 0.95f, 1, 1.0f, {0.85f, 0.85f, 0.8f, 1}, false, 1e9f,
     atlas::frontier},
    {fr::Thief, 36, 68, 0, 7, 1.0f, 10, 0.9f, {0.95f, 0.8f, 0.3f, 1}, false, 90,
     atlas::frontier},
    {0,22,42,4,6,1,3,1,{0.7f,0.3f,0.5f,1},false,35,atlas::expansion},
    {1,32,30,3,6,1,4,1,{0.5f,0.9f,0.5f,1},true,55,atlas::expansion},
    {2,38,50,10,6,1,4,0.8f,{0.5f,0.8f,0.9f,1},false,75,atlas::expansion},
    {3,28,48,8,6,1,3,1,{0.7f,0.5f,0.3f,1},true,65,atlas::expansion},
    {4,26,33,4,5,1,4,1,{0.5f,0.8f,1,1},true,80,atlas::expansion},
    {5,100,25,14,9,1.3f,8,0.3f,{0.6f,0.8f,0.9f,1},false,130,atlas::expansion},
    {6,18,38,4,5,1,3,1,{0.6f,0.8f,0.3f,1},false,50,atlas::expansion},
    {7,130,24,12,10,1.3f,9,0.3f,{0.6f,0.4f,0.7f,1},false,150,atlas::expansion},
    {8,36,45,7,6,1,5,0.8f,{1,0.4f,0.2f,1},false,110,atlas::expansion},
    {9,160,22,16,10,1.4f,12,0.25f,{0.7f,0.3f,0.1f,1},false,190,atlas::expansion},
};
constexpr int ELITE_XP = 6;

void puffs(Level &level, double x, double y, Color c, int n, float speed,
           float rise) {
  for (int i = 0; i < n && Drift::room(); i++) {
    float a = rng.range(0, TAU), v = rng.range(0.4f, 1) * speed;
    Drift *d = level.add<Drift>(x, y, std::cos(a) * v, std::sin(a) * v * 0.6f - rise,
                                atlas::pt::Puff, c, rng.range(0.6f, 1.0f));
    d->grow = 1.3f, d->scale = 0.6f, d->spin = rng.range(-1, 1);
  }
}

void chips(Level &level, double x, double y, float z, int icon, Color c, int n,
           double speed) {
  for (int i = 0; i < n && Debris::room(); i++)
    level.add<Chip>(x, y, z, icon, c, speed);
}

bool signature(Level &level, EnemyKind k, double x, double y, double body,
               float scale) {
  switch (k) {
  case EnemyKind::Skeleton:
    chips(level, x, y, 8 * scale, atlas::pt::Bone, col::white, 5, 1.1);
    puffs(level, x, y, Color{0.85f, 0.82f, 0.72f, 0.45f}, 3, 20, 4);
    return true;
  case EnemyKind::Iceback:
    chips(level, x, y, 8 * scale, atlas::pt::IceShard, col::white, 9, 1.4);
    level.add<Shockwave>(x, body, 3, 18 * scale, 0.3f,
                         Color{0.75f, 0.92f, 1, 0.9f}, false);
    return true;
  case EnemyKind::Sporeling:
  case EnemyKind::MycelialBrute:
    puffs(level, x, body, Color{0.78f, 0.95f, 0.55f, 0.5f},
          k == EnemyKind::Sporeling ? 5 : 9, 35 * scale, 6);
    return true;
  case EnemyKind::Burrower:
    chips(level, x, y, 4, atlas::pt::Rock, col::white, 4, 1.3);
    chips(level, x, y, 4, atlas::pt::Pebble, col::white, 4, 1.6);
    puffs(level, x, y, Color{0.62f, 0.5f, 0.38f, 0.5f}, 4, 30, 2);
    return true;
  default:
    return false;
  }
}

void flesh(Level &level, EnemyKind k, double x, double y, double body,
           float scale) {
  switch (k) {
  case EnemyKind::Knight:
    chips(level, x, y, 8 * scale, atlas::pt::Pebble, Color{0.8f, 0.85f, 0.95f, 1},
          4, 1.2);
    Spark::burst(level, x, body, col::white, 5, 80, false);
    break;
  case EnemyKind::Cultist:
  case EnemyKind::Necromancer:
    puffs(level, x, body, Color{0.35f, 0.15f, 0.45f, 0.55f}, 5, 25, 10);
    for (int i = 0; i < 2 && Drift::room(); i++) {
      Drift *d = level.add<Drift>(x, body, rng.range(-8, 8), -30,
                                  atlas::pt::Wisp, Color{0.75f, 0.55f, 1, 0.9f},
                                  0.9f);
      d->sway = 6, d->drag = 0.5f;
    }
    break;
  case EnemyKind::Thief:
    chips(level, x, y, 8, atlas::pt::Coin, col::white, 6, 1.3);
    break;
  case EnemyKind::MireLeech:
    puffs(level, x, y, Color{0.35f, 0.45f, 0.25f, 0.5f}, 3, 20, 2);
    break;
  default:
    break;
  }
}
} // namespace

Enemy::Enemy(const EnemyType &type, float hpMul, float damageMul)
    : type(type), damageMul(damageMul) {
  hp = maxHp = type.hp * hpMul;
  radius = type.radius;
  scale = type.scale;
  animT = rng.range(0, 10);
  seed = rng.next() | 1;
  side = seed & 2 ? 1 : -1;
}

const EnemyType &Enemy::typeOf(EnemyKind kind) { return TYPES[int(kind)]; }

std::unique_ptr<Enemy> Enemy::create(EnemyKind kind, float hpMul,
                                     float damageMul) {
  const EnemyType &t = typeOf(kind);
  std::unique_ptr<Enemy> e;
  if (kind >= EnemyKind::MireLeech)
    e = std::make_unique<ExpansionEnemy>(t,hpMul,damageMul,int(kind)-int(EnemyKind::MireLeech));
  else if (kind == EnemyKind::Ghost)
    e = std::make_unique<Ghost>(t, hpMul, damageMul);
  else if (kind == EnemyKind::Cyclops)
    e = std::make_unique<Cyclops>(t, hpMul, damageMul);
  else if (kind == EnemyKind::Wolf)
    e = std::make_unique<Wolf>(t, hpMul, damageMul);
  else if (kind == EnemyKind::Alpha)
    e = std::make_unique<AlphaWolf>(t, hpMul, damageMul);
  else if (kind == EnemyKind::Burrower)
    e = std::make_unique<Burrower>(t, hpMul, damageMul);
  else if (kind == EnemyKind::Necromancer)
    e = std::make_unique<Necromancer>(t, hpMul, damageMul);
  else if (kind == EnemyKind::Thief)
    e = std::make_unique<Thief>(t, hpMul, damageMul);
  else
    e = std::make_unique<Enemy>(t, hpMul, damageMul);
  e->plan = planOf(kind);
  return e;
}

float Enemy::roll() {
  seed ^= seed << 13, seed ^= seed >> 17, seed ^= seed << 5;
  return float(seed >> 8) / float(1 << 24);
}

void Enemy::voice(Sound &s, float volume, float jitter) const {
  if (level->game.screen.visible(x, y, 24))
    s.play(volume, jitter);
}

Sense Enemy::sense() const {
  const Player &p = *level->player;
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

void Enemy::act() {
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
  level->add<Orb>(x, bodyY(), xd / dd * 70, yd / dd * 70, 10 * damageMul)
      ->by = name();
  if (boss) {
    double turn = std::atan2(yd, xd) + PI / 10;
    for (int i = 0; i < 10; i++) {
      double a = turn + TAU * i / 10;
      level->add<Orb>(x, bodyY(), std::cos(a) * 45, std::sin(a) * 45,
                      8 * damageMul)->by = name();
    }
    level->add<Shockwave>(x, bodyY(), 6, 40, 0.4f, Color{0.75f, 0.35f, 1, 0.9f})
        ->spokes = 10;
  }
  Sound::orb.play(0.3f);
}

void Enemy::onAdded() { level->tag(this); }

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
      ((uintptr_t(this) >> 5) + level->ticks) & 1)
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
}

void Enemy::tick() {
  status.tick(DT, [&](float damage, Upgrade *src) {
    hurt(damage, 0, 0, 0, src, true);
    return !removed;
  });
  if (removed)
    return;
  act();
  if (!type.floats)
    traits.tick(*this);
  Player &p = *level->player;
  double xd = p.x - x, yd = p.y - y, dist = std::sqrt(xd * xd + yd * yd);
  float reach = type.radius + 5;
  if (dist < reach && dist > 1e-3 && !p.dead()) {
    x = p.x - xd / dist * reach;
    y = p.y - yd / dist * reach;
    p.felledBy = name();
    bool lands = p.invuln <= 0 && !p.god;
    float before = p.hp;
    p.hurt(type.damage * damageMul);
    if (lands)
      traits.landed(*this, std::max(before - p.hp, maxHp * 0.1f));
  }
}

void Enemy::makeElite(float health) {
  elite = true;
  traits.add(Trait(int(roll() * int(Trait::Count)) % int(Trait::Count)));
  hp = maxHp *= health;
  scale *= 1.35f;
  radius *= 1.3f;
}

void Enemy::split() {
  for (int i = 0; i < 2; i++) {
    auto e = Enemy::create(EnemyKind(kind()), 1, damageMul);
    e->hp = e->maxHp = maxHp / 8;
    e->x = x + (i ? 8 : -8), e->y = y, e->aware = true;
    level->add(std::move(e));
  }
  Spark::burst(*level, x, bodyY(), TRAITS[int(Trait::Splitting)].color, 8, 60, false);
}

void Enemy::makeBoss(float health, float damage) {
  boss = true;
  hp = maxHp = health;
  damageMul *= damage;
  scale *= 2.2f;
  radius *= 1.9f;
}

void Enemy::render(Screen &s) {
  if (boss) {
    float beat = 0.5f + 0.25f * std::sin(age * 5);
    s.ring(xp, yp, radius + 6, Color{0.9f, 0.15f, 0.2f, beat});
    s.ring(xp, yp, radius + 10, Color{0.9f, 0.15f, 0.2f, beat * 0.4f});
  }
  Color c = tint();
  traits.render(s, *this, radius, age);
  if (traits.any()) {
    Color a = traits.tone();
    c = {c.r * (0.7f + 0.3f * a.r), c.g * (0.7f + 0.3f * a.g),
         c.b * (0.7f + 0.3f * a.b), c.a};
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
}

void Enemy::renderShadow(Screen &s) { drawShadow(s, xp, yp, type.floats); }

Sprite Enemy::sprite() const {
  return type.art ? type.art(type.tile) : atlas::tile(type.tile);
}

const char *Enemy::name() const {
  static const char *const NAMES[] = {
      "a bat",          "a rat",           "a slime",
      "a scorpion",     "a spider",        "a ghost",
      "a cultist",      "a demon",         "a knight",
      "the cyclops",    "a wolf",          "an alpha wolf",
      "a burrower",     "a necromancer",   "a skeleton",
      "a treasure thief", "a mire leech", "a bog lantern", "a glass mantis",
      "a dust vulture", "a frost wisp", "an iceback", "a sporeling",
      "a mycelial brute", "an ember imp", "a cinder golem"
  };
  return NAMES[&type - TYPES];
}

int Enemy::kind() const { return int(&type - TYPES); }

void Enemy::hurt(float damage, double xd, double yd, float force,
                 Upgrade *src, bool periodic) {
  if (removed)
    return;
  double dd = std::sqrt(xd * xd + yd * yd);
  if (dd > 1e-5)
    xd /= dd, yd /= dd;
  else
    xd = yd = 0;
  if (!periodic)
    damage *= status.incoming();
  if (!periodic)
    damage = traits.incoming(*this, damage);
  if (src && !type.floats &&
      level->heightAt(level->player->x, level->player->y) > level->heightAt(x, y))
    damage *= 1.15f;
  bool crit = !periodic && rng.chance(0.08f + level->player->luck);
  if (crit)
    damage *= level->player->hero ? level->player->hero->edge() : 1.8f;
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
  TextParticle::pop(*level, x, bodyY(), int(std::lround(damage)),
                    crit ? col::gold : col::white);
  Sound::hit.play(0.35f);
  if (hp <= 0)
    die();
}

void Enemy::die() {
  status.clear();
  remove();
  level->kills++;
  level->slain[kind()]++;
  level->streak++, level->streakTime = 2.5f;
  level->bestStreak = std::max(level->bestStreak, level->streak);
  level->game.codex.slain[kind()]++;
  if (Player *p = level->player; p->hero) {
    p->hero->slay(*p, *this);
    for (auto &up : p->upgrades)
      if (up->level > 0)
        up->slay(*p, *this);
  }
  Sound::kill.play(0.35f, 0.12f);
  Gem::drop(*level, x, y - 3,
            type.xp * (elite ? ELITE_XP : 1) * level->xpMul *
                (1 + std::min(level->streak, 100) * 0.005f));
  if (elite && rng.chance(0.25f))
    level->add<Potion>(x + 6, y);
  if (elite)
    level->game.hitStop(0.035f);
  traits.died(*this, type.damage * damageMul * 2);
  if (boss) {
    level->add<Chest>(x, y);
    level->add<Shockwave>(x, bodyY(), 6, 90, 0.6f, Color{0.95f, 0.2f, 0.25f, 1})
        ->spokes = 16;
    Spark::burst(*level, x, bodyY(), type.blood, 30, 150, true);
    level->game.hitStop(0.12f);
    level->game.onExplosion();
    level->game.announce(is(EnemyKind::Alpha) ? "THE PACK LORD FALLS"
                                              : "THE LICH CRUMBLES");
  }
  if (rng.chance(1 / 150.0f))
    level->add<Potion>(x, y);
  if (gilded) {
    constexpr Color GOLD{1, 0.7f, 0.15f, 1};
    level->add<DeathEffect>(x, y, float(z), sprite(), scale, facing, GOLD,
                            DeathStyle::Gilded);
    level->add<Shockwave>(x, bodyY(), 4, 22 * scale, 0.3f, GOLD, false);
    for (int i = 0; i < 7 && Debris::room(); i++)
      level->add<Chip>(x, y, 6 * scale, atlas::pt::Coin, col::white, 1.3);
    return;
  }
  EnemyKind k = EnemyKind(kind());
  bool spirit = k == EnemyKind::Ghost || k == EnemyKind::BogLantern ||
                k == EnemyKind::FrostWisp;
  bool flame = k == EnemyKind::Demon || k == EnemyKind::EmberImp ||
               k == EnemyKind::CinderGolem;
  bool slime = k == EnemyKind::Slime;
  bool fallen = k == EnemyKind::Bat || k == EnemyKind::Scorpion ||
                k == EnemyKind::Spider || k == EnemyKind::GlassMantis ||
                k == EnemyKind::DustVulture;
  if (spirit || flame || slime || fallen) {
    DeathStyle style = spirit ? DeathStyle::Spirit : flame ? DeathStyle::Flame :
                       slime ? DeathStyle::Slime : DeathStyle::Fallen;
    if (DeathEffect::alive < 180)
      level->add<DeathEffect>(x, y, float(z), sprite(), scale, facing,
                              type.blood, style);
    if (spirit || slime) {
      for (int i = 0; i < 12 && Spark::alive < 500; i++)
        level->add<Spark>(x + rng.range(-6, 6) * scale, bodyY(),
                          rng.range(-15, 15), spirit ? -rng.range(25, 55) : rng.range(0, 8),
                          type.blood, rng.range(0.4f, 0.9f), rng.range(0.3f, 0.7f), false);
    } else if (flame) {
      for (int i = 0; i < 12; i++)
        FlameDebris::lick(*level, x + rng.range(-6, 6) * scale,
                          y + rng.range(-4, 4) * scale, rng.range(2, 12) * scale, 1);
      Spark::burst(*level, x, bodyY(), {1, 0.35f, 0.08f, 1}, 12, 70, true);
    }
  } else if (signature(*level, k, x, y, bodyY(), scale)) {
    if (DeathEffect::alive < 180 && k != EnemyKind::Sporeling &&
        k != EnemyKind::Iceback)
      level->add<DeathEffect>(x, y, float(z), sprite(), scale, facing,
                              type.blood, DeathStyle::Fallen);
  } else {
    flesh(*level, k, x, y, bodyY(), scale);
    Color blood = k == EnemyKind::Rat || k == EnemyKind::Cultist ||
                          k == EnemyKind::Knight || k == EnemyKind::Cyclops ||
                          k == EnemyKind::Wolf || k == EnemyKind::Alpha ||
                          k == EnemyKind::Necromancer || k == EnemyKind::Thief
                      ? Color{0.65f, 0.08f, 0.1f, 1}
                      : type.blood;
    Debris::gore(*level, x, y, 7 * scale, blood, true);
    Spark::burst(*level, x, bodyY(), blood, 6, 45, false);
  }
}
