#include "item/weapon/Recipe.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
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
};

namespace {
bool area(Form f) { return f == Form::Nova || f == Form::Field; }

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
  return r.count + (area(r.form) ? 0 : at(level, 3) + at(level, 5));
}

float RecipeWeapon::reach() const {
  return r.reach * (area(r.form) ? 1 + 0.3f * (at(level, 3) + at(level, 5)) : 1);
}

float RecipeWeapon::damage(const Player &p) const {
  return r.damage * (1 + 0.5f * (at(level, 2) + at(level, 5))) * p.might * power;
}

void RecipeWeapon::tick(Player &p) {
  if ((cd -= DT) > 0 || p.dead())
    return;
  cd = r.cooldown * (level >= 4 ? 0.75f : 1) * p.haste * pace;
  fireRecipe(r, p, this, damage(p), reach(), count());
}

void fireRecipe(const Recipe &r, Player &p, Upgrade *source, float damage,
                float reach, int count) {
  Level &level = *p.level;
  auto targets = level.nearest(p.x, p.y, std::max(reach, 240.0f), 8);
  double dx = p.xAim, dy = p.yAim;
  if (!targets.empty())
    dx = targets.front()->x - p.x, dy = targets.front()->bodyY() - p.bodyY();
  unit(dx, dy);
  double base = std::atan2(dy, dx);
  auto shot = [&](double x, double y, double a, int i) {
    level.add<RecipeShot>(r, x, y, std::cos(a), std::sin(a), damage, reach, i,
                          count)->source = source;
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
        Enemy *e = targets[size_t(i) % targets.size()];
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
  }
}

bool RecipeShot::hitBefore(entt::entity id) const {
  return std::find(hits.begin(), hits.end(), id) != hits.end();
}

void RecipeShot::strike(Enemy &e, double xd, double yd) {
  hits.push_back(e.id);
  unsigned p = r.payload;
  if (p & pay::Burn)
    e.afflict(StatusKind::Burn, 2.0f, damage * 0.25f, source);
  if (p & pay::Chill)
    e.afflict(StatusKind::Chill, 1.2f, 0.3f, source);
  if (p & pay::Poison)
    e.afflict(StatusKind::Poison, 3.0f, damage * 0.2f, source);
  if (p & pay::Mark)
    e.afflict(StatusKind::Marked, 2.5f, 0.15f, source);
  float knock = p & pay::Pull ? -70.0f :
                r.form == Form::Field ? 0.0f :
                r.form == Form::Orbit ? 60.0f : 40.0f;
  e.hurt(damage, xd, yd, knock, source);
  if (Spark::alive < 600)
    Spark::burst(*level, e.x, e.bodyY(), r.tint, 4, 60, false);
  if ((p & pay::Explode) && r.form != Form::Rain)
    blast(24, damage * 0.5f);
  if ((p & pay::Split) && !child)
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
  level->enemiesNear(x, y, radius + 16, [&](Enemy &e) {
    double ex = e.x - x, ey = e.bodyY() - y;
    float rr = radius + e.radius;
    if (ex * ex + ey * ey >= rr * rr)
      return;
    if (r.payload & pay::Burn)
      e.afflict(StatusKind::Burn, 2.0f, dmg * 0.25f, source);
    e.hurt(dmg, ex, ey, 70, source);
  });
}

void RecipeShot::steer(float range) {
  Enemy *target = nullptr;
  for (Enemy *e : level->nearest(x, y, range, 12))
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
  bool moving = r.form == Form::Bolt || r.form == Form::Volley ||
                r.form == Form::Seeker || r.form == Form::Orbit;
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
  Player &p = *level->player;
  if (p.dead())
    return remove();
  age += DT;
  if (age >= life) {
    if (burstOnEnd)
      blast(reach * 1.2f, damage * 6);
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
    level->enemiesNear(x, y, radius + 24, [&](Enemy &e) {
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
    level->enemiesNear(x, y, radius + 24, [&](Enemy &e) {
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
    level->enemiesNear(x, y, ring + 16, [&](Enemy &e) {
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
    level->enemiesNear(x, y, reach + 16, [&](Enemy &e) {
      double ex = e.x - x, ey = e.bodyY() - y;
      float rr = reach + e.radius;
      if (ex * ex + ey * ey < rr * rr)
        strike(e, ex, ey);
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
  case Form::Orbit: {
    float k = child ? 0.7f : 1.0f;
    s.color(c.alpha(0.35f));
    s.center(atlas::Glow, xp, yp, 14 * k / atlas::Glow.w);
    s.color(col::white);
    float rot = r.form == Form::Orbit ? age * 6 : float(std::atan2(dy, dx));
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
  }
}

void RecipeShot::renderLight(Screen &s) {
  float size = r.form == Form::Field ? reach * 2 :
               r.form == Form::Nova ? reach * 1.5f : 30;
  if (r.form == Form::Rain && pulse > 0)
    return;
  s.glow(xp, yp, size, r.tint * 0.12f);
}
