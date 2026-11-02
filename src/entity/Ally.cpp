#include "entity/Ally.hpp"

#include <cstring>

#include "entity/Player.hpp"
#include "item/weapon/Recipe.hpp"
#include "level/Level.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace {

using K = EnemyKind;

const Recipe DRONE_BOLT{"Drone Bolt", nullptr, "", "bolt", 50, Form::Bolt, pay::Pierce, 0, 1,
                        180, 300, 1, {1, 0.8f, 0.45f, 1}, atlas::pt::Coin};

const AllyDef ALLIES[] = {
    {"wolf", "Wolf Totem", K::Wolf, 40, 96, 0.7f, 8, 12, 0.9f, false, false,
     {0.75f, 0.85f, 1, 1}, nullptr},
    {"hawk", "Hawk Companion", K::DustVulture, 35, 140, 0.5f, 6, 10, 0.8f, true, false,
     {0.95f, 0.85f, 0.6f, 1}, nullptr},
    {"ghoul", "Ghoul Pack", K::Skeleton, 60, 88, 0.8f, 8, 14, 1, false, false,
     {0.6f, 0.9f, 0.55f, 1}, nullptr},
    {"skeleton", "Raise Dead", K::Skeleton, 30, 74, 0.9f, 8, 10, 1, false, false,
     {0.85f, 0.85f, 1, 1}, nullptr},
    {"drone", "Clockwork Drone", K::BogLantern, 30, 90, 0.8f, 90, 15, 0.8f, true, false,
     {1, 0.8f, 0.45f, 1}, &DRONE_BOLT},
    {"crew", "Powder Crew", K::Outlaw, 45, 70, 1.0f, 10, 12, 1, false, false,
     {0.9f, 0.75f, 0.6f, 1}, nullptr},
    {"decoy", "Mirror Image", K::Knight, 60, 0, 0, 0, 4, 1, false, true,
     {0.6f, 0.8f, 1, 1}, nullptr},
};
static_assert(std::size(ALLIES) == size_t(AllyKind::Count), "every AllyKind needs a row");

int nextSlot = 0;

Color soften(Color c, float a) {
  return {0.55f + 0.45f * c.r, 0.55f + 0.45f * c.g, 0.55f + 0.45f * c.b, a};
}

} // namespace

const AllyDef &allyDef(AllyKind kind) { return ALLIES[int(kind)]; }

AllyKind allyFor(const char *recipe) {
  for (size_t i = 0; i < std::size(ALLIES); i++)
    if (recipe && std::strcmp(ALLIES[i].recipe, recipe) == 0)
      return AllyKind(i);
  return AllyKind::Wolf;
}

Ally::Ally(AllyKind kind, double x, double y, float damage, float life, unsigned payload)
    : def(allyDef(kind)), damage(damage), life(life), payload(payload) {
  this->x = x, this->y = y;
  hp = maxHp = def.hp;
  speed = def.speed;
  scale = def.scale;
  radius = 5 * scale;
  animT = rng.range(0, 10);
}

void Ally::onAdded() { level->tag(this); }

Mob *Ally::pick() const {
  if (def.lure)
    return nullptr;
  const Player &p = *level->player;
  double hx = x - p.x, hy = y - p.y;
  if (hx * hx + hy * hy > LEASH * LEASH)
    return nullptr;
  for (Enemy *e : level->nearest(x, y, SIGHT, 1)) {
    double ex = e->x - p.x, ey = e->y - p.y;
    if (ex * ex + ey * ey < LEASH * LEASH && e->type.damage > 0)
      return e;
  }
  return nullptr;
}

void Ally::stride(double tx, double ty, float pace) {
  double xd = tx - x, yd = ty - y, d = std::sqrt(xd * xd + yd * yd);
  if (d < 2 || pace <= 0) {
    xa = ya = 0;
    return;
  }
  double step = std::min(double(pace * status.movement()), d / DT);
  xa = xd / d * step, ya = yd / d * step;
  x += xa * DT, y += ya * DT;
  if (std::abs(xa) > 1)
    facing = xa > 0 ? 1 : -1;
  if (!def.flies)
    level->collide(x, y, radius);
}

void Ally::tick() {
  age += DT, animT += DT;
  status.moving = xa * xa + ya * ya > 25;
  status.tick(DT, [&](float d, Upgrade *) {
    if ((hp -= d) <= 0)
      retire();
    return !removed;
  });
  if (removed)
    return;
  if ((life -= DT) <= 0)
    return retire();
  invuln = std::max(0.0f, invuln - DT), flash = std::max(0.0f, flash - DT * 5), bite -= DT;
  Player &p = *level->player;
  if (Mob *foe = pick()) {
    double xd = foe->x - x, yd = foe->y - y, d = std::max(1e-3, std::sqrt(xd * xd + yd * yd));
    xAim = xd / d, yAim = yd / d;
    double close = def.reach + foe->radius + radius;
    if (d > close)
      stride(foe->x - xAim * close * 0.8, foe->y - yAim * close * 0.8, def.speed);
    else
      xa = ya = 0;
    if (d <= close && bite <= 0 && def.shot) {
      level->add<RecipeShot>(*def.shot, x, bodyY(), xAim, yAim, damage, def.shot->reach)
          ->source = source;
      bite = def.every;
    } else if (d <= close && bite <= 0) {
      landHit(payload, *foe, xAim, yAim, damage, 40, source, this);
      if (payload & pay::Explode)
        blast(*foe);
      bite = def.every;
    }
  } else {
    double a = slot * 2.39996 + age * 0.6, home = HOME + slot % 4 * 12;
    stride(p.x + std::cos(a) * home, p.y + std::sin(a) * home * 0.7, def.speed * 1.2f);
  }
  if (!def.flies)
    bitten();
}

void Ally::blast(const Mob &at) {
  double bx = at.x, by = at.y;
  level->add<Shockwave>(bx, at.bodyY(), 3, 26, 0.3f, def.tint);
  level->enemiesNear(bx, by, 40, [&](Enemy &e) {
    double xd = e.x - bx, yd = e.y - by;
    if (xd * xd + yd * yd < 26 * 26 && &e != &at)
      e.hurt(damage * 0.5f, xd, yd, 60, source);
  });
}

void Ally::bitten() {
  level->enemiesNear(x, y, radius + 14, [&](Enemy &e) {
    if (invuln > 0 || removed || e.type.damage <= 0 || e.buried || !e.plan)
      return;
    double xd = x - e.x, yd = y - e.y;
    double r = radius + e.radius + 2;
    if (xd * xd + yd * yd < r * r)
      strike(e.type.damage * e.damageMul * e.status.outgoing(), xd, yd, 60);
  });
}

void Ally::strike(float dmg, double xd, double yd, float force, Upgrade *) {
  if (removed || invuln > 0)
    return;
  hp -= dmg, flash = 1, invuln = GRACE;
  double d = std::sqrt(xd * xd + yd * yd);
  if (d > 1e-3 && force > 0 && !def.lure)
    x += xd / d * force * 0.08, y += yd / d * force * 0.08;
  if (hp <= 0)
    retire();
}

void Ally::retire() {
  if (removed)
    return;
  Spark::burst(*level, x, bodyY(), def.tint, 8, 40, true);
  remove();
}

Sprite Ally::body() const {
  if (def.lure)
    return level->player->body();
  const EnemyType &t = Enemy::typeOf(def.look);
  return t.art ? t.art(t.tile) : atlas::tile(t.tile);
}

void Ally::render(Screen &s) {
  float fade = std::min(1.0f, life) * std::min(1.0f, age * 4);
  int lift = def.flies ? 6 + int(std::lround(std::sin(age * 5) * 2)) : 0;
  s.ring(xp, yp, radius + 4, def.tint.alpha(0.3f * fade));
  drawBody(s, xp, yp - lift, body(), def.flies, status.moving ? 1.0f : 0.0f,
           soften(def.tint, (def.lure ? 0.55f : 1.0f) * fade), flash, col::white);
}

void Ally::renderShadow(Screen &s) { drawShadow(s, xp, yp, def.flies); }

Ally *summon(Level &level, AllyKind kind, double x, double y, float damage, float life,
             unsigned payload, Upgrade *source) {
  Ally *a = level.add<Ally>(kind, x, y, damage, life, payload);
  a->source = source;
  a->slot = nextSlot++;
  Spark::burst(level, x, y - 7, a->def.tint, 6, 40, true);
  return a;
}
