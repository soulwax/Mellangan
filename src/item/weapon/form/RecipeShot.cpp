#include "item/weapon/Recipe.hpp"

#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "item/weapon/form/FormBehavior.hpp"
#include "level/Level.hpp"
#include "particle/Arc.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

RecipeShot::RecipeShot(const Recipe &r, double x, double y, double dx, double dy,
                       float damage, float reach, int index, int of)
    : r(r), damage(damage), reach(reach), ox(x), oy(y), dx(dx), dy(dy),
      index(index), of(std::max(1, of)) {
  this->x = x, this->y = y;
  unit(this->dx, this->dy);
  radius = 5;
  hitsLeft = r.payload & pay::Pierce ? 3 : 1;
  life = form().life(*this);
}

Mob *RecipeShot::wielder() const {
  if (source && source->holder)
    return source->holder;
  return hostile() ? nullptr : level->player;
}

bool RecipeShot::hitBefore(entt::entity who) const {
  return std::find(hits.begin(), hits.end(), who) != hits.end();
}

void RecipeShot::hit(Mob &e, double xd, double yd, float dmg, float knock) {
  landHit(r.payload, e, xd, yd, dmg, knock, source, wielder());
}

void landHit(unsigned p, Mob &e, double xd, double yd, float dmg, float knock,
             Upgrade *source, Mob *w) {
  if ((p & pay::Shatter) && e.status.power(StatusKind::Chill) > 0)
    dmg *= 1.5f, e.status.effects[int(StatusKind::Chill)] = {};
  if (p & pay::Burn)
    e.afflict(StatusKind::Burn, 2.0f, dmg * 0.25f, source);
  if (p & pay::Chill)
    e.afflict(StatusKind::Chill, 1.2f, 0.3f, source);
  if (p & pay::Poison)
    e.afflict(StatusKind::Poison, 3.0f, dmg * 0.2f, source);
  if (p & pay::Mark)
    e.afflict(StatusKind::Marked, 2.5f, 0.15f, source);
  if (p & pay::Stagger)
    e.afflict(StatusKind::Chill, 0.5f * (1 + (w ? w->bonus.juggernaut / 100 : 0)), 0.65f, source),
        knock += knock > 0 ? 120 : 0;
  if (p & pay::Bleed)
    e.afflict(StatusKind::Bleed, 3.0f, dmg * 0.15f, source);
  if (p & pay::Weaken)
    e.afflict(StatusKind::Weaken, 3.0f, 0.25f, source);
  if (p & pay::Fear)
    e.afflict(StatusKind::Fear, 1.5f, 1, source);
  e.strike(dmg, xd, yd, knock, source);
  if ((p & pay::Execute) && !e.dead() && !e.asPlayer() && !e.bossLike() &&
      e.hp < e.maxHp * 0.08f)
    e.strike(e.hp + 1, xd, yd, 0, source);
  if (p & pay::Leech)
    if (w && !w->dead())
      w->hp = std::min(w->maxHp, w->hp + std::min(2.0f, dmg * 0.04f));
}

void RecipeShot::chain(Mob &from) {
  for (Mob *e : level->nearestFoes(hostile(), from.x, from.y, 90, 4))
    if (e != &from && !hitBefore(e->id)) {
      hits.push_back(e->id);
      Mob *w = wielder();
      hit(*e, e->x - from.x, e->y - from.y, damage * 0.5f * (w ? w->arcs : 1), 30);
      level->add<Arc>(std::vector<ArcNode>{{from.x, from.bodyY()}, {e->x, e->bodyY()}},
                      0.2f, r.tint);
      return;
    }
}

void RecipeShot::touch(float rad) {
  level->foes(hostile(), x, y, rad + 24, [&](Mob &e) {
    if (removed || hitBefore(e.id))
      return;
    double ex = e.x - x, ey = e.bodyY() - y;
    float rr = rad + e.radius;
    if (ex * ex + ey * ey < rr * rr)
      strike(e, ex, ey);
  });
}

void RecipeShot::strike(Mob &e, double xd, double yd) {
  hits.push_back(e.id);
  unsigned p = r.payload;
  float knock = p & pay::Pull ? -70.0f : form().knock();
  hit(e, xd, yd, damage, knock);
  if (Spark::alive < 600)
    Spark::burst(*level, e.x, e.bodyY(), r.tint, 4, 60, false);
  if ((p & pay::Explode) && form().explodes())
    blast(24, damage * 0.5f);
  if ((p & pay::Chain) && !child)
    chain(e);
  if ((p & pay::Split) && !child && form().flies())
    for (int k = -1; k <= 1; k += 2) {
      double a = std::atan2(dy, dx) + k * 1.2;
      RecipeShot *s = level->add<RecipeShot>(r, e.x, e.bodyY(), std::cos(a),
                                             std::sin(a), damage * 0.5f, reach);
      s->child = true, s->life = 1.5f, s->source = source;
      s->hits.push_back(e.id);
    }
}

void RecipeShot::blast(float rad, float dmg) {
  level->add<Shockwave>(x, y, 3, rad, 0.3f, r.tint);
  if (Spark::alive < 600)
    Spark::burst(*level, x, y, r.tint, 8, rad * 2, true);
  level->foes(hostile(), x, y, rad + 16, [&](Mob &e) {
    double ex = e.x - x, ey = e.bodyY() - y;
    float rr = rad + e.radius;
    if (ex * ex + ey * ey >= rr * rr)
      return;
    hit(e, ex, ey, dmg, 70);
  });
}

void RecipeShot::steer(float range) {
  Mob *target = nullptr;
  for (Mob *e : level->nearestFoes(hostile(), x, y, range, 12))
    if (!hitBefore(e->id)) {
      target = e;
      break;
    }
  if (!target)
    return;
  double ax = target->x - x, ay = target->bodyY() - y;
  unit(ax, ay);
  double k = std::min(1.0f, 5 * DT);
  dx = dx * (1 - k) + ax * k, dy = dy * (1 - k) + ay * k;
  unit(dx, dy);
}

void RecipeShot::tick() {
  Mob *w = wielder();
  if (!w || w->dead())
    return remove();
  Mob &p = *w;
  age += DT;
  if (age >= life) {
    if (burstOnEnd)
      blast(reach * 1.2f, damage * 6);
    form().expire(*this);
    return remove();
  }
  form().trail(*this);
  form().tick(*this, p, (child ? r.speed * 1.3f : r.speed) * (1 + p.bonus.shotSpeed / 100));
}

void RecipeShot::render(Screen &s) { form().render(*this, s); }

void RecipeShot::renderLight(Screen &s) { form().light(*this, s); }

const FormBehavior &RecipeShot::form() const { return FormBehavior::of(r.form); }
