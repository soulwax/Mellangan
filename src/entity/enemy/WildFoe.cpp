#include "entity/enemy/WildFoe.hpp"

#include "audio/Sound.hpp"
#include "entity/Ability.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/enemy/ai/Mend.hpp"
#include "entity/pickup/ItemDrop.hpp"
#include "entity/projectile/Arrow.hpp"
#include "entity/projectile/Orb.hpp"
#include "particle/debris/SmokeDebris.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "particle/Arc.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace {

constexpr Color HEX{0.55f, 0.85f, 0.35f, 1}, TONGUE{0.9f, 0.45f, 0.5f, 1},
                WATER{0.45f, 0.7f, 0.9f, 1}, VINE{0.4f, 0.7f, 0.3f, 1},
                SPORE{0.75f, 0.6f, 0.85f, 1}, SNOW{0.92f, 0.96f, 1, 1},
                FROST{0.55f, 0.85f, 1, 1}, LEECH{0.5f, 0.95f, 0.6f, 1},
                CRYSTAL{0.8f, 0.65f, 1, 1}, MOON{0.75f, 0.8f, 1, 1},
                SAND{0.9f, 0.72f, 0.42f, 1}, SALT{0.95f, 0.95f, 0.9f, 1},
                GLASS{0.6f, 0.95f, 0.9f, 1}, STORM{0.65f, 0.75f, 1, 1},
                HASTE{1, 0.6f, 0.25f, 1}, WARD{0.45f, 0.7f, 1, 1},
                FIRE{1, 0.5f, 0.2f, 1}, ASH{0.55f, 0.5f, 0.5f, 1},
                SOIL{0.5f, 0.38f, 0.25f, 1}, GLARE{1, 0.25f, 0.3f, 1},
                STONE{0.55f, 0.52f, 0.5f, 0.85f};

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

bool sight(Enemy &e, double near, double far, double &xd, double &yd) {
  Player &p = *e.level->player;
  xd = p.x - e.x, yd = p.bodyY() - e.bodyY();
  double d = std::sqrt(xd * xd + yd * yd);
  if (p.dead() || d < near || d > far)
    return false;
  xd /= d, yd /= d;
  return true;
}

void snowball(Enemy &e) {
  double xd, yd;
  if (!sight(e, 50, 170, xd, yd))
    return;
  Orb *o = e.level->add<Orb>(e.x, e.bodyY(), xd * 95, yd * 95, 8 * e.power());
  o->by = e.name(), o->tone = SNOW, o->chill = 0.85f, o->size = 1.9f;
}

void frostshot(Enemy &e) {
  double xd, yd;
  if (!sight(e, 0, 240, xd, yd))
    return;
  Arrow *a = e.level->add<Arrow>(e.x, e.bodyY(), xd * 150, yd * 150, 6 * e.power(), 1);
  a->hostile = true, a->chill = 0.35f;
  Spark::burst(*e.level, e.x + xd * 6, e.bodyY() + yd * 6, FROST, 3, 30, false);
}

void glass(Enemy &e) {
  double xd, yd;
  if (!sight(e, 0, 210, xd, yd))
    return;
  double aim = std::atan2(yd, xd);
  for (int i = -1; i <= 1; i++) {
    double a = aim + i * 0.25;
    Orb *o = e.level->add<Orb>(e.x, e.bodyY(), std::cos(a) * 105, std::sin(a) * 105,
                               5 * e.power());
    o->by = e.name(), o->tone = GLASS, o->size = 0.8f, o->bounces = 1;
  }
}

void lightning(Enemy &e) {
  Player &p = *e.level->player;
  double xd, yd;
  if (!sight(e, 0, 220, xd, yd))
    return;
  Blast *b = e.level->add<Blast>(p.x, p.y, 10 * e.power(), true);
  b->tone = STORM, b->radius = 22, b->by = e.name();
}

void javelin(Enemy &e) {
  double xd, yd;
  if (!sight(e, 40, 200, xd, yd))
    return;
  Arrow *a = e.level->add<Arrow>(e.x, e.bodyY(), xd * 190, yd * 190, 14 * e.power(), 1);
  a->hostile = true;
  e.castCount = 0;
}

void plant(Enemy &e) {
  Enemy *near[24];
  int totems = 0;
  for (int i = 0, n = e.level->gather(e.x, e.y, 140, near, 24); i < n; i++)
    totems += near[i]->is(EnemyKind::WarTotem);
  if (totems >= 2)
    return;
  float a = e.roll() * TAU;
  e.level->spawner.spawn(EnemyKind::WarTotem, e.x + std::cos(a) * 20, e.y + std::sin(a) * 14);
  e.level->add<Shockwave>(e.x, e.bodyY(), 3, 24, 0.35f, HASTE, false);
}

void meteor(Enemy &e) {
  Player &p = *e.level->player;
  double xd, yd;
  if (!sight(e, 0, 220, xd, yd))
    return;
  Blast *b = e.level->add<Blast>(p.x, p.y, 16 * e.power(), true);
  b->life = b->maxLife = 2, b->radius = 34, b->tone = FIRE, b->by = e.name();
}

void dig(Enemy &e) {
  float a = e.roll() * TAU;
  double gx = e.x + std::cos(a) * 18, gy = e.y + std::sin(a) * 12;
  e.level->spawner.spawn(EnemyKind::Skeleton, gx, gy);
  e.level->add<Shockwave>(gx, gy, 3, 20, 0.4f, SOIL, false);
  Spark::burst(*e.level, gx, gy - 2, SOIL, 8, 50, true);
}

void pickaxe(Enemy &e) {
  double xd, yd;
  if (!sight(e, 30, 160, xd, yd))
    return;
  Arrow *a = e.level->add<Arrow>(e.x, e.bodyY(), xd * 140, yd * 140, 6 * e.power(), 1);
  a->hostile = true;
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
    {.cast = {snowball, 3.5f, 170, SNOW}, .casts = 1},
    {.aura = true},
    {.drains = true},
    {.guarded = true, .reflects = true, .shatters = true},
    {},
    {.nocturnal = true, .blinks = true},
    {.lurks = true},
    {.cast = {frostshot, 2.6f, 240, FROST}, .casts = 1},
    {.wyrm = true},
    {.skittish = true},
    {.salty = true},
    {.cast = {glass, 2.4f, 210, GLASS}, .casts = 1},
    {.cast = {lightning, 3.5f, 220, STORM}, .casts = 1},
    {.cast = {javelin, 1.5f, 200, SAND}, .casts = 1},
    {.cast = {plant, 7, 200, HASTE}, .casts = 1},
    {.totem = true},
    {.trail = true},
    {.veil = true, .scorches = true},
    {.blind = true},
    {.shatters = true},
    {},
    {.cast = {meteor, 5, 220, FIRE}, .casts = 1},
    {.guarded = true},
    {.drake = true},
    {.nocturnal = true, .stalker = true},
    {.cast = {dig, 6, 200, SOIL}, .casts = 1, .robber = true},
    {},
    {.lurks = true, .clings = true},
    {.ore = true},
    {.cast = {pickaxe, 3, 160, SOIL}, .casts = 1, .pilfers = true},
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
  if (wild.nocturnal && level->night.dawn)
    return remove();
  if (!wild.waterborn || leader != entt::null) {
    buried = (wild.lurks && !wild.waterborn) || wild.wyrm || wild.blind;
    heard = level->noiseAt;
    return;
  }
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
  if (wild.nocturnal && level->night.dawn) {
    Spark::burst(*level, x, bodyY(), MOON, 6, 40, true);
    return remove();
  }
  glintCool -= DT;
  if (glint)
    reflect();
  if (p.dead())
    return;
  double xd = p.x - x, yd = p.y - y, d2 = xd * xd + yd * yd;
  float reach = type.radius + p.radius + 4;
  if (wild.aura && d2 < 40 * 40)
    p.afflict(StatusKind::Chill, 0.3f, 0.3f);
  if (wild.drains)
    leech(p, std::sqrt(d2));
  if (wild.blinks && aware)
    blink(p, std::sqrt(d2));
  if (wild.wyrm)
    burrow(p);
  if (wild.skittish && !fled && hp < maxHp * 0.3f)
    fled = true, afflict(StatusKind::Fear, 3.5f, 1);
  if (wild.salty && xWant * xWant + yWant * yWant > 4 && (saltTime -= DT) <= 0) {
    saltTime = 0.6f;
    level->add<FrostPatch>(x, y, true)->tone = SALT;
  }
  if (wild.totem)
    pulse();
  if (wild.trail && xWant * xWant + yWant * yWant > 4 && (saltTime -= DT) <= 0) {
    saltTime = 0.5f;
    FrostPatch *f = level->add<FrostPatch>(x, y, true);
    f->tone = FIRE, f->kind = StatusKind::Burn, f->power = 2.5f * power();
    f->life = f->maxLife = 4;
  }
  if (wild.blind)
    listen();
  if (wild.drake)
    fly(p, std::sqrt(d2));
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

void WildFoe::leech(Player &p, double d) {
  if (drainTime <= 0) {
    if ((drainCool -= DT) > 0 || d > 100)
      return;
    drainTime = 2, drainTick = 0;
  }
  if (d > 130) {
    drainTime = 0, drainCool = 3;
    Spark::burst(*level, x, bodyY(), LEECH, 4, 30, true);
    return;
  }
  if ((drainTick -= DT) <= 0) {
    drainTick = 0.25f;
    level->add<Arc>(std::vector<ArcNode>{{x, bodyY()}, {p.x, p.bodyY()}}, 0.26f, LEECH);
    float sip = 2.5f * power();
    p.felledBy = name();
    p.hurt(sip, true);
    hp = std::min(maxHp, hp + sip);
  }
  if ((drainTime -= DT) <= 0)
    drainCool = 4;
}

void WildFoe::blink(Player &p, double d) {
  if ((blinkTime -= DT) > 0 || d < 60 || d > 200)
    return;
  blinkTime = 3.5f + 1.5f * roll();
  Spark::burst(*level, x, bodyY(), MOON, 5, 40, true);
  double a = std::atan2(y - p.y, x - p.x) + (roll() < 0.5f ? 1.3 : -1.3);
  x = p.x + std::cos(a) * 34, y = p.y + std::sin(a) * 34;
  level->collide(x, y, radius);
  Spark::burst(*level, x, bodyY(), MOON, 5, 40, true);
}

void WildFoe::reflect() {
  glint = false;
  double xd, yd;
  if (glintCool > 0 || !sight(*this, 0, 260, xd, yd))
    return;
  glintCool = 0.6f;
  Orb *o = level->add<Orb>(x + xd * radius, bodyY() + yd * radius, xd * 110, yd * 110,
                           6 * power());
  o->by = name(), o->tone = CRYSTAL, o->size = 0.9f;
  Spark::burst(*level, x + xd * radius, bodyY() + yd * radius, CRYSTAL, 4, 50, false);
}

void WildFoe::burrow(Player &p) {
  if (buried && level->ticks % 6 == 0 && Debris::room())
    level->add<SmokeDebris>(x, y, 1);
  if ((wyrmTime -= DT) > 0)
    return;
  if (!buried) {
    buried = true, eruptions = 0, wyrmTime = 1.2f;
    Spark::burst(*level, x, bodyY(), SAND, 14, 70, true);
    return;
  }
  if (eruptions == 3) {
    buried = false, wyrmTime = 4.5f;
    level->add<Shockwave>(x, y, 6, 50, 0.5f, SAND, false);
    voice(Sound::erupt, 0.5f);
    return;
  }
  eruptions++, wyrmTime = 1.6f;
  x = p.x, y = p.y;
  Blast *b = level->add<Blast>(p.x, p.y, type.damage * power(), true);
  b->tone = SAND, b->radius = 30, b->by = name();
}

void WildFoe::pulse() {
  x = xHome, y = yHome;
  if ((pulseTime -= DT) > 0)
    return;
  pulseTime = 1.5f;
  bool ward = pulses++ % 2;
  level->enemiesNear(x, y, 72, [&](Enemy &e) {
    if (&e == this || e.is(EnemyKind::WarTotem))
      return;
    if (ward)
      e.shield(1);
    else
      e.frenzy = std::max(e.frenzy, 1.6f);
  });
  level->add<Shockwave>(x, bodyY(), 4, 72, 0.45f, (ward ? WARD : HASTE).alpha(0.4f), false);
}

void WildFoe::listen() {
  if (level->noiseAt <= heard)
    return;
  heard = level->noiseAt;
  double xd = level->xNoise - x, yd = level->yNoise - y;
  if (xd * xd + yd * yd > (buried ? 160 * 160 : 240 * 240))
    return;
  frenzy = std::max(frenzy, 1.0f);
  if (!buried)
    return;
  buried = false, aware = true;
  level->add<Shockwave>(x, bodyY(), 5, 44, 0.45f, type.blood, false);
  voice(Sound::erupt, 0.45f);
}

void WildFoe::fly(Player &p, double d) {
  if (restTime > 0) {
    x = xRest, y = yRest, z = 0;
    if ((restTime -= DT) <= 0) {
      flightTime = 10;
      level->add<Shockwave>(x, y, 6, 44, 0.4f, FIRE, false);
    }
    return;
  }
  if ((flightTime -= DT) <= 0) {
    restTime = 3, xRest = x, yRest = y, z = 0;
    level->add<Shockwave>(x, y, 6, 54, 0.5f, ASH, false);
    return;
  }
  if ((breathTime -= DT) <= 0 && d < 120 && !p.dead()) {
    breathTime = 2.5f;
    breathe();
  }
}

void WildFoe::breathe() {
  double xd, yd;
  if (!sight(*this, 0, 140, xd, yd))
    return;
  double aim = std::atan2(yd, xd);
  for (int i = 0; i < 7; i++) {
    double a = aim + (i - 3) * 0.15, v = 120 + 30 * roll();
    Orb *o = level->add<Orb>(x + xd * radius, bodyY() + yd * radius, std::cos(a) * v,
                             std::sin(a) * v, 5 * power());
    o->by = name(), o->tone = FIRE, o->burn = 2 * power(), o->size = 1;
  }
}

bool WildFoe::veiled() const {
  const Player &p = *level->player;
  double xd = p.x - x, yd = p.y - y;
  return wild.veil && xd * xd + yd * yd >= 60 * 60;
}

Color WildFoe::tint() const {
  if (wild.veil)
    return col::white.alpha(veiled() ? 0.14f : 0.75f);
  if (wild.blind && buried)
    return Color{0.6f, 0.6f, 0.7f, 0.7f};
  if (wild.clings && buried)
    return STONE;
  return wild.aura ? col::white.alpha(0.6f + 0.15f * std::sin(animT * 2)) : col::white;
}

void WildFoe::render(Screen &s) {
  if (wild.stalker && flash <= 0) {
    if (std::fmod(animT, 2.6f) < 0.15f)
      return;
    s.rect(xp - 3, yp - 8, 2, 1, GLARE.alpha(0.9f));
    s.rect(xp + 2, yp - 8, 2, 1, GLARE.alpha(0.9f));
    return;
  }
  if (!buried || wild.blind || wild.clings)
    return Enemy::render(s);
  if (wild.wyrm) {
    float k = 0.5f + 0.5f * std::sin(animT * 4);
    s.ring(xp, yp - 2, 8 + 3 * k, SAND.alpha(0.45f));
    s.ring(xp, yp - 2, 14 + 2 * k, SAND.alpha(0.2f));
    return;
  }
  if (!wild.waterborn) {
    if (std::fmod(animT, 3.2f) < 0.18f)
      return;
    s.rect(xp - 3, yp - 8, 2, 1, col::gold.alpha(0.8f));
    s.rect(xp + 2, yp - 8, 2, 1, col::gold.alpha(0.8f));
    return;
  }
  float k = 0.5f + 0.5f * std::sin(animT * 3);
  s.ring(xp, yp - 2, 4 + 2 * k, WATER.alpha(0.35f));
  s.ring(xp + 5, yp, 2 + k, WATER.alpha(0.25f));
}

void WildFoe::renderShadow(Screen &s) {
  if (wild.stalker)
    return;
  if (!buried || wild.blind || wild.clings)
    Enemy::renderShadow(s);
}

float WildFoe::contactBonus(const Player &p) const {
  return wild.backstab && (x - p.x) * p.facing < 0 ? 1.5f : 1.0f;
}

float WildFoe::shell(double xd, double yd) {
  if (wild.wyrm)
    return buried ? 0 : 1.5f;
  if (wild.drake)
    return restTime > 0 ? 1.5f : 1;
  if (wild.veil && veiled())
    return 0.5f;
  if (!wild.guarded || (xd == 0 && yd == 0))
    return 1;
  double facing = xd * xMark + yd * yMark;
  glint |= wild.reflects && facing < -0.3;
  return facing < -0.3 ? 0.3f : facing > 0.3 ? 1.3f : 1.0f;
}

void WildFoe::bit(Player &p) {
  if (wild.chills)
    p.afflict(StatusKind::Chill, 1.5f, 0.35f);
  if (wild.scorches)
    p.afflict(StatusKind::Burn, 3, 2 * power());
  int &ore = p.materials[int(Mat::Ore)];
  if (wild.pilfers && !fled && ore > 0) {
    ore--, stolen++, fled = true;
    afflict(StatusKind::Fear, 8, 1);
    Spark::burst(*level, x, bodyY(), MATERIALS[int(Mat::Ore)].color, 6, 40, true);
  }
}

void WildFoe::bequeath() {
  if (!level->rules.loot)
    return;
  auto spill = [&](Drop d) {
    float a = roll() * TAU, r = 8 + roll() * 18;
    level->add<ItemDrop>(x, y, d, x + std::cos(a) * r, y + std::sin(a) * r * 0.7f);
  };
  if (wild.robber)
    spill({DropKind::Gold, level->monsterLevel(x, y) * 6, {}});
  for (int i = 0; i < (wild.ore ? 2 : 0) + stolen; i++)
    spill({DropKind::Material, 1, {}, int(Mat::Ore)});
}

void WildFoe::die() {
  bequeath();
  if (wild.bramble)
    level->add<Bramble>(x, y);
  for (int i = 0; wild.shatters && i < 3; i++) {
    float a = TAU * i / 3 + roll();
    level->spawner.spawn(EnemyKind(kind() + 1), x + std::cos(a) * 10, y + std::sin(a) * 10);
  }
  if (wild.shatters)
    level->add<Shockwave>(x, bodyY(), 4, 34, 0.4f, type.blood, false);
  Enemy::die();
}
