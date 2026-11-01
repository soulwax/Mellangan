#include "item/weapon/Recipe.hpp"

#include "item/weapon/form/FormBehavior.hpp"

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

RecipeWeapon::RecipeWeapon(const Recipe &r)
    : Upgrade(r.name, atlas::arsenal(r.icon), {}, true), r(r) {
  owner = r.owner;
  int add = int(std::lround(r.damage * 0.5f));
  bool wide = FormBehavior::of(r.form).area();
  text = {r.pitch, fmt::format("+{} damage", add),
          wide ? std::string("+30% radius") : fmt::format("+1 {}", r.unit),
          "25% faster",
          wide ? fmt::format("+{} damage, +30% radius", add)
               : fmt::format("+{} damage, +1 {}", add, r.unit)};
  for (int i = 0; i < MAX_LEVEL; i++)
    desc[i] = text[i].c_str();
}

int RecipeWeapon::count() const {
  return r.count + (FormBehavior::of(r.form).area() ? 0 : at(level, 3) + at(level, 5) + evolved);
}

float RecipeWeapon::reach() const {
  return r.reach * (FormBehavior::of(r.form).area() ? 1 + 0.3f * (at(level, 3) + at(level, 5)) +
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
  FormBehavior::of(r.form).fire(
      {r, p, source, damage, reach, count, targets, dx, dy, std::atan2(dy, dx)});
}

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
  for (Mob *e : level->nearestFoes(hostile(), from.x, from.y, 90, 4))
    if (e != &from && !hitBefore(e->id)) {
      hits.push_back(e->id);
      hit(*e, e->x - from.x, e->y - from.y, damage * 0.5f, 30);
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

void RecipeShot::blast(float radius, float dmg) {
  level->add<Shockwave>(x, y, 3, radius, 0.3f, r.tint);
  if (Spark::alive < 600)
    Spark::burst(*level, x, y, r.tint, 8, radius * 2, true);
  level->foes(hostile(), x, y, radius + 16, [&](Mob &e) {
    double ex = e.x - x, ey = e.bodyY() - y;
    float rr = radius + e.radius;
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
  double tx = target->x - x, ty = target->bodyY() - y;
  unit(tx, ty);
  double k = std::min(1.0f, 5 * DT);
  dx = dx * (1 - k) + tx * k, dy = dy * (1 - k) + ty * k;
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
  form().tick(*this, p, child ? r.speed * 1.3f : r.speed);
}

void RecipeShot::render(Screen &s) { form().render(*this, s); }

void RecipeShot::renderLight(Screen &s) { form().light(*this, s); }

const FormBehavior &RecipeShot::form() const { return FormBehavior::of(r.form); }
