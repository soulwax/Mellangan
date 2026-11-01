#include "item/weapon/Recipe.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Arc.hpp"
#include "particle/Drift.hpp"
#include "particle/Mark.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

const std::vector<Recipe> RECIPES = {
    {"Ember Lance", "MAGE", "A searing lance pierces and ignites foes", "lance",
     0, Form::Bolt, pay::Burn | pay::Pierce, 14, 1.6f, 260, 330, 1,
     {1, 0.55f, 0.2f, 1}, atlas::pt::Ember},
    {"Hoarfrost Fan", "MAGE", "Fans out ice shards that chill foes", "shard", 1,
     Form::Volley, pay::Chill, 8, 1.8f, 150, 260, 5, {0.6f, 0.85f, 1, 1},
     atlas::pt::IceShard},
    {"Arcane Orbit", "MAGE", "Arcane orbs circle you and mark foes", "orb", 2,
     Form::Orbit, pay::Mark, 9, 4.0f, 34, 3.2f, 2, {0.78f, 0.55f, 1, 1},
     atlas::pt::ArcaneStar},
    {"Starfall", "MAGE", "Stars crash down on distant foes", "star", 3,
     Form::Rain, pay::Explode, 26, 3.2f, 200, 0, 3, {1, 0.92f, 0.6f, 1},
     atlas::pt::FallingStar},
    {"Void Rift", "MAGE", "A rift drags foes inward and grinds them", "rift", 4,
     Form::Field, pay::Pull, 5, 5.0f, 48, 0, 1, {0.55f, 0.3f, 0.95f, 1},
     atlas::pt::Wisp},
    {"Seeker Runes", "MAGE", "Runes hunt foes and split on impact", "rune", 5,
     Form::Seeker, pay::Split, 11, 2.4f, 240, 150, 2, {0.5f, 0.9f, 1, 1},
     atlas::pt::Rune},
    {"Flame Ring", "MAGE", "A ring of fire bursts outward", "ring", 6,
     Form::Nova, pay::Burn, 12, 3.0f, 90, 0, 1, {1, 0.45f, 0.15f, 1},
     atlas::pt::Ember},
    {"Lionheart Cleave", "KNIGHT", "A broad cleave staggers foes before you",
     "sweep", 7, Form::Cleave, pay::Stagger, 18, 1.5f, 52, 0, 1,
     {1, 0.86f, 0.5f, 1}, atlas::pt::Crescent},
    {"Bastion Shields", "KNIGHT", "Iron shields circle you and batter foes back",
     "shield", 8, Form::Orbit, pay::Stagger, 10, 4.5f, 30, 2.6f, 2,
     {0.8f, 0.85f, 0.95f, 1}, atlas::pt::Coin},
    {"Hawk Talon", "RANGER", "A curved blade flies out and wheels back",
     "talon", 9, Form::Boomerang, pay::Pierce, 14, 2.0f, 170, 240, 1,
     {0.75f, 0.95f, 0.6f, 1}, atlas::pt::Feather},
    {"Bear Trap", "RANGER", "Hidden jaws snap shut and pin foes", "trap", 10,
     Form::Trap, pay::Stagger, 30, 2.6f, 34, 0, 1, {0.85f, 0.8f, 0.7f, 1},
     atlas::pt::Pebble},
    {"Caustic Flask", "ALCHEMIST", "Lobbed flasks burst into choking poison",
     "flask", 11, Form::Lob, pay::Poison, 16, 2.2f, 200, 0, 1,
     {0.55f, 0.95f, 0.35f, 1}, atlas::pt::Bubble},
    {"Witchfire Phial", "ALCHEMIST", "Phials of witchfire splash, burn and mark",
     "phial", 12, Form::Lob, pay::Burn | pay::Mark, 14, 2.6f, 200, 0, 2,
     {0.95f, 0.45f, 0.9f, 1}, atlas::pt::Ember},
    {"Ball Lightning", "STORMCALLER", "Crackling orbs hunt foes and arc onward",
     "orb", 13, Form::Seeker, pay::Chain, 10, 2.2f, 240, 140, 2,
     {0.55f, 0.75f, 1, 1}, atlas::pt::Wisp},
    {"Arc Lance", "STORMCALLER", "A bolt lances a line of foes and leaps on",
     "lance", 14, Form::Beam, pay::Chain, 16, 1.8f, 180, 0, 1,
     {0.6f, 0.85f, 1, 1}, atlas::pt::Rune},
    {"Soul Harvest", "REAPER", "A wide reaping arc that drinks their life",
     "sweep", 15, Form::Cleave, pay::Leech, 16, 1.6f, 58, 0, 1,
     {0.75f, 0.3f, 0.55f, 1}, atlas::pt::Crescent},
    {"Bone Wheel", "REAPER", "A wheel of bone carves out and back, poisoning",
     "wheel", 16, Form::Boomerang, pay::Poison, 12, 2.2f, 160, 220, 1,
     {0.95f, 0.9f, 0.8f, 1}, atlas::pt::Bone},
    {"Lance of Dawn", "PRIESTESS", "A ray of dawn sears and marks a line of foes",
     "ray", 17, Form::Beam, pay::Burn | pay::Mark, 15, 2.0f, 190, 0, 1,
     {1, 0.9f, 0.55f, 1}, atlas::pt::HolyGlint},
    {"Seraph Wings", "PRIESTESS", "Feathers of light circle you and mend you",
     "feather", 18, Form::Orbit, pay::Leech, 8, 4.5f, 40, 2.4f, 3,
     {1, 0.97f, 0.85f, 1}, atlas::pt::Feather},
    {"Crescent Cut", "DUELIST", "Quick twin arcs that mark foes for the kill",
     "cut", 19, Form::Cleave, pay::Mark, 12, 1.2f, 46, 0, 2,
     {1, 0.8f, 0.9f, 1}, atlas::pt::Crescent},
    {"Flicker Darts", "DUELIST", "Darts seek out foes and pierce through", "dart",
     20, Form::Seeker, pay::Pierce, 9, 1.6f, 220, 200, 3, {0.95f, 0.95f, 1, 1},
     atlas::pt::Petal},
    {"Thornwhip", "WARDEN", "A lashing vine poisons and staggers foes", "lash",
     21, Form::Cleave, pay::Poison | pay::Stagger, 14, 1.5f, 60, 0, 1,
     {0.5f, 0.85f, 0.4f, 1}, atlas::pt::Leaf},
    {"Root Snare", "WARDEN", "Seeds sprout roots that seize passing foes",
     "seed", 22, Form::Trap, pay::Chill | pay::Poison, 24, 2.8f, 40, 0, 2,
     {0.45f, 0.75f, 0.3f, 1}, atlas::pt::MapleLeaf},
    {"Clockwork Mine", "TINKERER", "Mines tick under your feet and blow apart",
     "mine", 23, Form::Trap, pay::Burn, 34, 2.4f, 44, 0, 1,
     {1, 0.7f, 0.35f, 1}, atlas::pt::Coin},
    {"Flash Grenade", "TINKERER", "A lobbed grenade dazzles and marks foes",
     "grenade", 24, Form::Lob, pay::Mark | pay::Stagger, 18, 2.4f, 210, 0, 1,
     {1, 0.95f, 0.7f, 1}, atlas::pt::Pebble},
    {"Prophecy Ray", "ORACLE", "Rays of foresight chill and mark foes", "ray",
     25, Form::Beam, pay::Mark | pay::Chill, 13, 1.8f, 210, 0, 2,
     {0.75f, 0.65f, 1, 1}, atlas::pt::ArcaneStar},
    {"Fate Wheel", "ORACLE", "Runes of fate circle you and arc to their kin",
     "rune", 26, Form::Orbit, pay::Chain, 9, 4.2f, 38, 2.8f, 2,
     {0.7f, 0.6f, 1, 1}, atlas::pt::Rune},
    {"Chain Shot", "CORSAIR", "Chained shot tears through and staggers foes",
     "shot", 27, Form::Bolt, pay::Pierce | pay::Stagger, 20, 1.9f, 240, 300, 1,
     {0.7f, 0.72f, 0.78f, 1}, atlas::pt::Rock},
    {"Riptide", "CORSAIR", "A riptide drags foes in and chills them", "tide",
     28, Form::Field, pay::Pull | pay::Chill, 6, 5.0f, 46, 0, 1,
     {0.4f, 0.85f, 0.9f, 1}, atlas::pt::Bubble},
};

namespace {
bool area(Form f) { return f == Form::Nova || f == Form::Field; }
bool flies(Form f) {
  return f == Form::Bolt || f == Form::Volley || f == Form::Seeker ||
         f == Form::Boomerang;
}

void unit(double &dx, double &dy) {
  double len = std::hypot(dx, dy);
  if (len < 0.001)
    dx = 1, dy = 0;
  else
    dx /= len, dy /= len;
}
} // namespace

RecipeWeapon::RecipeWeapon(const Recipe &r)
    : Upgrade(r.name, atlas::arsenal(r.icon), {}, true), r(r) {
  owner = r.owner;
  int add = int(std::lround(r.damage * 0.5f));
  bool wide = area(r.form);
  text = {r.pitch, fmt::format("+{} damage", add),
          wide ? std::string("+30% radius") : fmt::format("+1 {}", r.unit),
          "25% faster",
          wide ? fmt::format("+{} damage, +30% radius", add)
               : fmt::format("+{} damage, +1 {}", add, r.unit)};
  for (int i = 0; i < MAX_LEVEL; i++)
    desc[i] = text[i].c_str();
}

int RecipeWeapon::count() const {
  return r.count + (area(r.form) ? 0 : at(level, 3) + at(level, 5) + evolved);
}

float RecipeWeapon::reach() const {
  return r.reach * (area(r.form) ? 1 + 0.3f * (at(level, 3) + at(level, 5)) +
                                        0.25f * evolved
                                  : 1);
}

float RecipeWeapon::damage(const Mob &p) const {
  return r.damage * (1 + 0.5f * (at(level, 2) + at(level, 5))) * p.might * power;
}

void RecipeWeapon::tick(Mob &p) {
  if ((cd -= DT) > 0 || p.dead())
    return;
  cd = r.cooldown * (level >= 4 ? 0.75f : 1) * p.haste * pace;
  fireRecipe(r, p, this, damage(p), reach(), count());
}

void fireRecipe(const Recipe &r, Mob &p, Upgrade *source, float damage,
                float reach, int count) {
  Level &level = *p.level;
  auto targets = level.nearestFoes(p, p.x, p.y, std::max(reach, 240.0f), 8);
  double dx = p.xAim, dy = p.yAim;
  if (!targets.empty())
    dx = targets.front()->x - p.x, dy = targets.front()->bodyY() - p.bodyY();
  unit(dx, dy);
  double base = std::atan2(dy, dx);
  auto shot = [&](double x, double y, double a, int i) {
    RecipeShot *s = level.add<RecipeShot>(r, x, y, std::cos(a), std::sin(a),
                                          damage, reach, i, count);
    s->source = source;
    return s;
  };
  switch (r.form) {
  case Form::Bolt:
    for (int i = 0; i < count; i++)
      shot(p.x, p.bodyY(), base + (i - (count - 1) * 0.5) * 0.12, i);
    Sound::cast.play(0.2f);
    break;
  case Form::Volley:
    for (int i = 0; i < count; i++)
      shot(p.x, p.bodyY(),
           base + (count > 1 ? (i / double(count - 1) - 0.5) * 0.9 : 0), i);
    Sound::cast.play(0.2f);
    break;
  case Form::Seeker:
    for (int i = 0; i < count; i++)
      shot(p.x, p.bodyY(), base + (i - (count - 1) * 0.5) * 0.7, i);
    Sound::orb.play(0.2f);
    break;
  case Form::Orbit:
    for (int i = 0; i < count; i++)
      shot(p.x, p.bodyY(), 0, i);
    Sound::orb.play(0.25f);
    break;
  case Form::Nova:
    shot(p.x, p.bodyY(), 0, 0);
    Sound::nova.play(0.3f);
    break;
  case Form::Rain:
    for (int i = 0; i < count; i++) {
      double x, y;
      if (!targets.empty()) {
        Mob *e = targets[size_t(i) % targets.size()];
        x = e->x, y = e->bodyY();
      } else {
        float a = rng.range(0, TAU), k = rng.range(0.3f, 1.0f) * reach;
        x = p.x + std::cos(a) * k, y = p.bodyY() + std::sin(a) * k;
      }
      shot(x + rng.range(-8, 8), y + rng.range(-8, 8), 0, i);
    }
    Sound::cast.play(0.25f);
    break;
  case Form::Field: {
    double x = p.x + dx * 60, y = p.bodyY() + dy * 60;
    if (!targets.empty())
      x = targets.front()->x, y = targets.front()->bodyY();
    shot(x, y, 0, 0);
    Sound::orb.play(0.3f);
    break;
  }
  case Form::Cleave:
    for (int i = 0; i < count; i++)
      shot(p.x, p.bodyY(), base, i)->age = -0.16f * i;
    Sound::slash.play(0.25f);
    break;
  case Form::Boomerang:
    for (int i = 0; i < count; i++)
      shot(p.x, p.bodyY(), base + (i - (count - 1) * 0.5) * 0.35, i);
    Sound::axe.play(0.2f);
    break;
  case Form::Lob:
    for (int i = 0; i < count; i++) {
      double x = p.x + dx * reach * 0.6, y = p.bodyY() + dy * reach * 0.6;
      if (!targets.empty()) {
        Mob *e = targets[size_t(i) % targets.size()];
        x = e->x, y = e->bodyY();
      }
      RecipeShot *s = shot(p.x, p.bodyY(), base, i);
      s->tx = x + (i ? rng.range(-14, 14) : 0);
      s->ty = y + (i ? rng.range(-10, 10) : 0);
    }
    Sound::axe.play(0.2f, 0.06f, 0.8f);
    break;
  case Form::Trap:
    for (int i = 0; i < count; i++) {
      float a = rng.range(0, TAU), k = i ? rng.range(14, 30) : 0;
      shot(p.x + std::cos(a) * k, p.y + std::sin(a) * k * 0.7f, 0, i);
    }
    Sound::axe.play(0.15f, 0.06f, 1.4f);
    break;
  case Form::Beam:
    for (int i = 0; i < count; i++)
      shot(p.x, p.bodyY(), base + (i - (count - 1) * 0.5) * 0.18, i);
    Sound::cast.play(0.25f, 0.06f, 1.3f);
    break;
  }
}

RecipeShot::RecipeShot(const Recipe &r, double x, double y, double dx, double dy,
                       float damage, float reach, int index, int of)
    : r(r), damage(damage), reach(reach), ox(x), oy(y), dx(dx), dy(dy),
      index(index), of(std::max(1, of)) {
  this->x = x, this->y = y;
  unit(this->dx, this->dy);
  radius = 5;
  hitsLeft = r.payload & pay::Pierce ? 3 : 1;
  switch (r.form) {
  case Form::Bolt:
  case Form::Volley:
    life = reach / std::max(1.0f, r.speed);
    break;
  case Form::Orbit:
    life = 3.2f;
    break;
  case Form::Nova:
    life = 0.45f;
    break;
  case Form::Rain:
    life = 0.6f;
    break;
  case Form::Field:
    life = 3.0f;
    break;
  case Form::Seeker:
    life = 3.0f;
    break;
  case Form::Cleave:
    life = 0.24f;
    break;
  case Form::Boomerang:
    life = std::min(4.0f, 2.4f * reach / std::max(1.0f, r.speed));
    break;
  case Form::Lob:
    life = 0.55f;
    break;
  case Form::Trap:
    life = 10;
    break;
  case Form::Beam:
    life = 0.28f;
    break;
  }
}

Mob *RecipeShot::wielder() const {
  if (source && source->holder)
    return source->holder;
  return source && source->hostile() ? nullptr : level->player;
}

bool RecipeShot::hitBefore(entt::entity id) const {
  return std::find(hits.begin(), hits.end(), id) != hits.end();
}

void RecipeShot::hit(Mob &e, double xd, double yd, float dmg, float knock) {
  unsigned p = r.payload;
  if (p & pay::Burn)
    e.afflict(StatusKind::Burn, 2.0f, dmg * 0.25f, source);
  if (p & pay::Chill)
    e.afflict(StatusKind::Chill, 1.2f, 0.3f, source);
  if (p & pay::Poison)
    e.afflict(StatusKind::Poison, 3.0f, dmg * 0.2f, source);
  if (p & pay::Mark)
    e.afflict(StatusKind::Marked, 2.5f, 0.15f, source);
  if (p & pay::Stagger)
    e.afflict(StatusKind::Chill, 0.5f, 0.65f, source), knock += 120;
  e.strike(dmg, xd, yd, knock, source);
  if (p & pay::Leech)
    if (Mob *w = wielder(); w && !w->dead())
      w->hp = std::min(w->maxHp, w->hp + std::min(2.0f, dmg * 0.04f));
}

void RecipeShot::chain(Mob &from) {
  for (Mob *e : level->nearestFoes(source && source->hostile(), from.x, from.y, 90, 4))
    if (e != &from && !hitBefore(e->id)) {
      hits.push_back(e->id);
      hit(*e, e->x - from.x, e->y - from.y, damage * 0.5f, 30);
      level->add<Arc>(std::vector<ArcNode>{{from.x, from.bodyY()}, {e->x, e->bodyY()}},
                      0.2f, r.tint);
      return;
    }
}

void RecipeShot::touch(float rad) {
  level->foes(source && source->hostile(), x, y, rad + 24, [&](Mob &e) {
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
  float knock = p & pay::Pull ? -70.0f :
                r.form == Form::Field ? 0.0f :
                r.form == Form::Orbit ? 60.0f : 40.0f;
  hit(e, xd, yd, damage, knock);
  if (Spark::alive < 600)
    Spark::burst(*level, e.x, e.bodyY(), r.tint, 4, 60, false);
  if ((p & pay::Explode) && r.form != Form::Rain)
    blast(24, damage * 0.5f);
  if ((p & pay::Chain) && !child)
    chain(e);
  if ((p & pay::Split) && !child && flies(r.form))
    for (int k = -1; k <= 1; k += 2) {
      double a = std::atan2(dy, dx) + k * 1.2;
      RecipeShot *s = level->add<RecipeShot>(r, e.x, e.bodyY(), std::cos(a),
                                             std::sin(a), damage * 0.5f, reach);
      s->child = true, s->life = 1.5f, s->source = source;
      s->hits.push_back(e.id);
    }
}

void RecipeShot::blast(float radius, float dmg) {
  level->add<Shockwave>(x, y, 3, radius, 0.3f, r.tint);
  if (Spark::alive < 600)
    Spark::burst(*level, x, y, r.tint, 8, radius * 2, true);
  level->foes(source && source->hostile(), x, y, radius + 16, [&](Mob &e) {
    double ex = e.x - x, ey = e.bodyY() - y;
    float rr = radius + e.radius;
    if (ex * ex + ey * ey >= rr * rr)
      return;
    hit(e, ex, ey, dmg, 70);
  });
}

void RecipeShot::steer(float range) {
  Mob *target = nullptr;
  for (Mob *e : level->nearestFoes(source && source->hostile(), x, y, range, 12))
    if (!hitBefore(e->id)) {
      target = e;
      break;
    }
  if (!target)
    return;
  double tx = target->x - x, ty = target->bodyY() - y;
  unit(tx, ty);
  double k = std::min(1.0f, 5 * DT);
  dx = dx * (1 - k) + tx * k, dy = dy * (1 - k) + ty * k;
  unit(dx, dy);
}

void RecipeShot::trail() {
  bool moving = flies(r.form) || r.form == Form::Orbit;
  if (moving && rng.chance(0.4f) && Drift::room()) {
    Drift *d = level->add<Drift>(x, y, -dx * 20 + rng.range(-6, 6),
                                 -dy * 20 + rng.range(-6, 6), r.particle,
                                 r.tint.alpha(0.6f), rng.range(0.2f, 0.35f));
    d->scale = child ? 0.4f : 0.6f, d->spin = rng.range(-4, 4);
  } else if (r.form == Form::Field && rng.chance(0.5f) && Drift::room()) {
    float a = rng.range(0, TAU);
    Drift *d = level->add<Drift>(x + std::cos(a) * reach, y + std::sin(a) * reach,
                                 -std::cos(a) * reach * 1.6f,
                                 -std::sin(a) * reach * 1.6f, r.particle,
                                 r.tint.alpha(0.7f), 0.5f);
    d->scale = 0.6f, d->spin = 3, d->drag = 0;
  }
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
    if (r.form == Form::Lob)
      x = tx, y = ty, blast(28, damage);
    return remove();
  }
  trail();
  float speed = child ? r.speed * 1.3f : r.speed;
  switch (r.form) {
  case Form::Seeker:
    steer(260);
    [[fallthrough]];
  case Form::Bolt:
  case Form::Volley: {
    x += dx * speed * DT, y += dy * speed * DT;
    level->foes(source && source->hostile(), x, y, radius + 24, [&](Mob &e) {
      if (removed || hitBefore(e.id))
        return;
      double ex = e.x - x, ey = e.bodyY() - y;
      float rr = radius + e.radius;
      if (ex * ex + ey * ey >= rr * rr)
        return;
      strike(e, dx, dy);
      if (--hitsLeft <= 0)
        remove();
    });
    break;
  }
  case Form::Orbit: {
    double a = age * r.speed + TAU * index / of;
    x = p.x + std::cos(a) * reach, y = p.bodyY() + std::sin(a) * reach * 0.8;
    dx = -std::sin(a), dy = std::cos(a);
    if ((pulse -= DT) <= 0)
      pulse = 0.6f, hits.clear();
    level->foes(source && source->hostile(), x, y, radius + 24, [&](Mob &e) {
      if (hitBefore(e.id))
        return;
      double ex = e.x - x, ey = e.bodyY() - y;
      float rr = radius + e.radius;
      if (ex * ex + ey * ey < rr * rr)
        strike(e, ex, ey);
    });
    break;
  }
  case Form::Nova: {
    float ring = reach * std::min(1.0f, age / 0.35f);
    level->foes(source && source->hostile(), x, y, ring + 16, [&](Mob &e) {
      if (hitBefore(e.id))
        return;
      double ex = e.x - x, ey = e.bodyY() - y, d = std::hypot(ex, ey);
      if (d < ring + e.radius)
        strike(e, ex, ey);
    });
    break;
  }
  case Form::Rain:
    if (age >= 0.5f && pulse == 0) {
      pulse = 1;
      blast(26, damage);
      level->add<Mark>(x, y, 1.5f, atlas::Glow, r.tint.alpha(0.12f),
                       60.0f / atlas::Glow.w);
    }
    break;
  case Form::Field:
    if ((pulse -= DT) > 0)
      break;
    pulse = 0.5f;
    hits.clear();
    level->foes(source && source->hostile(), x, y, reach + 16, [&](Mob &e) {
      double ex = e.x - x, ey = e.bodyY() - y;
      float rr = reach + e.radius;
      if (ex * ex + ey * ey < rr * rr)
        strike(e, ex, ey);
    });
    break;
  case Form::Cleave:
    x = p.x, y = p.bodyY();
    if (age < 0)
      break;
    level->foes(source && source->hostile(), x, y, reach + 24, [&](Mob &e) {
      if (hitBefore(e.id))
        return;
      double ex = e.x - x, ey = e.bodyY() - y, d = std::hypot(ex, ey);
      if (d < reach + e.radius && ex * dx + ey * dy > d * 0.3)
        strike(e, ex, ey);
    });
    break;
  case Form::Boomerang: {
    if (!back && age >= reach / std::max(1.0f, r.speed))
      back = true, hits.clear();
    if (back) {
      double hx = p.x - x, hy = p.bodyY() - y, d = std::hypot(hx, hy);
      if (d < 10)
        return remove();
      dx = hx / d, dy = hy / d;
    }
    float v = back ? speed * 1.25f : speed;
    x += dx * v * DT, y += dy * v * DT;
    touch(radius + 2);
    break;
  }
  case Form::Lob: {
    float k = std::min(1.0f, age / life);
    x = ox + (tx - ox) * k, y = oy + (ty - oy) * k;
    break;
  }
  case Form::Trap: {
    if (age < 0.6f)
      break;
    bool sprung = false;
    level->foes(source && source->hostile(), x, y, 40, [&](Mob &e) {
      double ex = e.x - x, ey = e.y - y;
      float rr = 10 + e.radius;
      sprung |= ex * ex + ey * ey < rr * rr;
    });
    if (sprung) {
      blast(reach, damage);
      return remove();
    }
    break;
  }
  case Form::Beam:
    if (pulse != 0)
      break;
    pulse = 1;
    tx = ox + dx * reach, ty = oy + dy * reach;
    level->foes(source && source->hostile(), (ox + tx) / 2, (oy + ty) / 2,
                reach / 2 + 24, [&](Mob &e) {
                  double ex = e.x - ox, ey = e.bodyY() - oy;
                  double b = std::clamp(ex * dx + ey * dy, 0.0, double(reach));
                  double qx = ex - dx * b, qy = ey - dy * b;
                  float rr = 7 + e.radius;
                  if (qx * qx + qy * qy < rr * rr)
                    strike(e, dx, dy);
                });
    break;
  }
}

void RecipeShot::render(Screen &s) {
  Color c = r.tint;
  switch (r.form) {
  case Form::Bolt:
  case Form::Volley:
  case Form::Seeker:
  case Form::Boomerang:
  case Form::Orbit: {
    float k = child ? 0.7f : 1.0f;
    s.color(c.alpha(0.35f));
    s.center(atlas::Glow, xp, yp, 14 * k / atlas::Glow.w);
    s.color(col::white);
    float rot = r.form == Form::Orbit       ? age * 6
                : r.form == Form::Boomerang ? age * 14
                                            : float(std::atan2(dy, dx));
    s.particle(r.particle, xp, yp, c, k, rot);
    break;
  }
  case Form::Nova: {
    float k = std::min(1.0f, age / 0.35f), ring = reach * k,
          a = 1 - std::max(0.0f, (age - 0.25f) / 0.2f);
    s.ring(xp, yp, ring, c.alpha(0.8f * a), 2);
    s.ring(xp, yp, ring * 0.8f, c.alpha(0.35f * a));
    for (int i = 0; i < 10; i++) {
      float t = TAU * i / 10 + age * 2;
      s.particle(r.particle, xp + std::cos(t) * ring, yp + std::sin(t) * ring,
                 c.alpha(a), 0.8f, t);
    }
    break;
  }
  case Form::Rain: {
    float k = std::min(1.0f, age / 0.5f);
    if (k < 1) {
      s.ring(xp, yp, 26 * (1.6f - 0.6f * k), c.alpha(0.25f + 0.5f * k));
      s.particle(r.particle, xp + 30 * (1 - k), yp - 140 * (1 - k * k), c, 1.4f,
                 2.4f);
    }
    break;
  }
  case Form::Field: {
    float fade = std::min({1.0f, age * 4, (life - age) * 3});
    s.ring(xp, yp, reach, c.alpha(0.6f * fade));
    s.ring(xp, yp, reach * 0.6f, c.alpha(0.3f * fade));
    for (int i = 0; i < 6; i++) {
      float t = -age * 2.2f + TAU * i / 6;
      s.particle(r.particle, xp + std::cos(t) * reach * 0.8f,
                 yp + std::sin(t) * reach * 0.8f, c.alpha(0.8f * fade), 1, t);
    }
    s.color(c.alpha(0.3f * fade));
    s.center(atlas::Glow, xp, yp, reach * (1.1f + 0.1f * std::sin(age * 8)) /
                                      atlas::Glow.w);
    s.color(col::white);
    break;
  }
  case Form::Cleave: {
    if (age < 0)
      break;
    float k = age / life, side = index % 2 ? -1.0f : 1.0f,
          a0 = float(std::atan2(dy, dx));
    auto at = [&](float t) { return a0 + side * (1.2f - 2.4f * t); };
    for (int j = 0; j < 6; j++) {
      float a1 = at(k * j / 6), a2 = at(k * (j + 1) / 6);
      s.line(x + std::cos(a1) * reach, y + std::sin(a1) * reach,
             x + std::cos(a2) * reach, y + std::sin(a2) * reach, 2,
             c.alpha(0.6f * (1 - k) * (j + 1) / 6));
    }
    for (int j = 0; j < 4; j++) {
      float t = k - j * 0.07f;
      if (t < 0)
        break;
      float a = at(t), rr = reach * 0.75f;
      s.particle(r.particle, x + std::cos(a) * rr, y + std::sin(a) * rr,
                 c.alpha((1 - j / 4.0f) * (1 - k * 0.5f)), 1.2f - j * 0.15f,
                 a + 1.57f);
    }
    break;
  }
  case Form::Lob: {
    float k = std::min(1.0f, age / life);
    s.ring(tx, ty, 10 + 18 * (1 - k), c.alpha(0.2f + 0.4f * k));
    s.particle(r.particle, x, y - std::sin(k * 3.14159f) * 38, c, 1.1f, age * 9);
    break;
  }
  case Form::Trap: {
    float fade = std::min({1.0f, age * 4, (life - age) * 2});
    s.particle(r.particle, x, y - 2, c.alpha(fade), 0.9f, 0);
    if (age > 0.6f)
      s.ring(x, y, 8 + 2 * std::sin(age * 6), c.alpha(0.35f * fade));
    break;
  }
  case Form::Beam: {
    if (pulse == 0)
      break;
    float a = 1 - age / life;
    s.line(ox, oy, tx, ty, 1 + 3 * a, c.alpha(0.7f * a));
    s.line(ox, oy, tx, ty, 1, col::white.alpha(a));
    for (int j = 1; j <= 5; j++) {
      float t = j / 5.0f;
      s.particle(r.particle, ox + (tx - ox) * t, oy + (ty - oy) * t, c.alpha(a),
                 0.7f, age * 4 + j);
    }
    break;
  }
  }
}

void RecipeShot::renderLight(Screen &s) {
  float size = r.form == Form::Field ? reach * 2 :
               r.form == Form::Nova ? reach * 1.5f : 30;
  if ((r.form == Form::Rain && pulse > 0) || (r.form == Form::Cleave && age < 0))
    return;
  if (r.form == Form::Beam) {
    if (pulse != 0)
      s.glow((ox + tx) / 2, (oy + ty) / 2, reach, r.tint * 0.1f);
    return;
  }
  if (r.form == Form::Trap)
    size = 16;
  s.glow(xp, yp, size, r.tint * 0.12f);
}
