#include "level/Level.hpp"
#include "item/Upgrade.hpp"

#include <spdlog/spdlog.h>

#include <cstring>

#include "Game.hpp"
#include "Prof.h"
#include "Workers.hpp"
#include "audio/Sound.hpp"
#include "entity/Merchant.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/Chest.hpp"
#include "hero/Hero.hpp"
#include "level/Encounter.hpp"
#include "level/ExpansionEncounter.hpp"
#include "level/Flavour.hpp"
#include "particle/Arc.hpp"
#include "particle/Drift.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"
#include "particle/VisualDetail.hpp"

namespace {

template <void (Enemy::*Step)()>
void pass(Workers &workers, std::vector<Enemy *> &list) {
  workers.run(
      int(list.size()), 256,
      [](void *ctx, int begin, int end) {
        Enemy **each = static_cast<Enemy **>(ctx);
        for (int i = begin; i < end; i++)
          (each[i]->*Step)();
      },
      list.data());
}

} // namespace

Level::~Level() = default;

Level::Level(Game &game, RunRules rules) : game(game), rules(std::move(rules)) {
  cellStart.assign(GRID * GRID + 1, 0);
  cellFill.assign(GRID * GRID, 0);
  player = add<Player>();
  flush();
  Bounty::roll(bounties, 3);
  if (this->rules.omen == RunRules::ROLL_OMEN ? rng.chance(0.85f) : this->rules.omen >= 0) {
    omen = this->rules.omen >= 0 ? this->rules.omen : rng.irange(0, OMEN_COUNT - 1);
    handicaps += OMENS[omen].handicap;
  }
}

const Omen &Level::portent() const { return omen >= 0 ? OMENS[omen] : CALM; }

SpriteObject *Level::spawn(std::unique_ptr<SpriteObject> p) {
  std::lock_guard<std::mutex> lock(addMutex);
  p->level = this;
  particles.push_back(std::move(p));
  return particles.back().get();
}

Entity *Level::add(std::unique_ptr<Entity> e) {
  std::lock_guard<std::mutex> lock(addMutex);
  e->level = this;
  e->id = reg.create();
  pending.push_back(std::move(e));
  return pending.back().get();
}

void Level::flush() {
  reg.destroy(dead.begin(), dead.end());
  dead.clear();

  for (size_t i = 0; i < pending.size(); i++) {
    if (pending[i]->removed) {
      if (reg.valid(pending[i]->id))
        reg.destroy(pending[i]->id);
      continue;
    }
    std::unique_ptr<Entity> e = std::move(pending[i]);
    Entity *raw = e.get();
    reg.emplace<Owned>(raw->id, std::move(e));
    raw->onAdded();
  }
  pending.clear();
  allies.clear();
  each<Ally>([&](Ally &a) { allies.push_back(&a); });
}

void Level::tick() {
  time += DT, ticks++;
  prof_begin(PROF_HOUSEKEEP);
  if (!player->dead() && (!night.dawn || !rules.endsAtDawn))
    night.tick();
  if (rules.loot && !player->dead())
    Merchant::visit(*this);
  if (!player->dead() && (!night.dawn || rules.dayRate > 0))
    spawner.tick(), explore(), hunt();
  climate();
  weather();
  ambience();
  buildGrid();
  if (!player->dead() && !night.dawn)
    events.tick(*this);
  prof_end(PROF_HOUSEKEEP);
  prof_begin(PROF_THINK);
  thinkers.clear();
  each<Enemy>([&](Enemy &e) { thinkers.push_back(&e); });
  Workers &workers = Workers::shared();
  pass<&Enemy::think>(workers, thinkers);
  pass<&Enemy::move>(workers, thinkers);
  pass<&Enemy::push>(workers, thinkers);
  pass<&Enemy::settle>(workers, thinkers);
  prof_end(PROF_THINK);
  prof_begin(PROF_ENTITIES);
  for (auto [id, o] : reg.view<Owned>().each())
    if (!o.self->removed)
      o.self->tick();
  prof_end(PROF_ENTITIES);
  prof_begin(PROF_PARTICLES);
  for (size_t i = 0; i < particles.size(); i++)
    if (!particles[i]->removed)
      particles[i]->tick();
  size_t alive = 0;
  for (auto &p : particles)
    if (!p->removed)
      particles[alive++] = std::move(p);
  particles.resize(alive);
  prof_end(PROF_PARTICLES);
  prof_begin(PROF_HOUSEKEEP);
  flush();
  prof_end(PROF_HOUSEKEEP);
}

namespace {
uint64_t structureKey(const Structure *s) {
  return uint64_t(uint32_t(s->rx)) << 32 | uint32_t(s->ry);
}
} // namespace

bool Level::visited(const Structure &s) const {
  return visits.contains(structureKey(&s));
}

bool Level::unvisited(const Structure *s, const void *level) {
  return !static_cast<const Level *>(level)->visited(*s);
}

void Level::ambience() {
  int rx = world::regionOf(int(std::floor(player->x / 16))),
      ry = world::regionOf(int(std::floor(player->y / 16)));
  Structure s;
  for (int y = ry - 1; y <= ry + 1; y++)
    for (int x = rx - 1; x <= rx + 1; x++) {
      if (!world::structureIn(x, y, s) || !ENCOUNTERS[int(s.kind)].ambient)
        continue;
      double xd = s.centerX() - player->x, yd = s.centerY() - player->y;
      if (xd * xd + yd * yd < 380 * 380)
        ENCOUNTERS[int(s.kind)].ambient(this, &s);
    }
}

void Level::climate() {
  BiomeId here = world::biomeUnder(player->x, player->y);
  const Flavour &f = FLAVOURS[here];
  const Color &g = AIRS[here].grade;
  float k = 1 - std::exp(-0.9f * DT);
  air = {lerpf(air.r, g.r, k), lerpf(air.g, g.g, k), lerpf(air.b, g.b, k), 1};
  const Omen *o = omen >= 0 ? &OMENS[omen] : nullptr;
  step = f.step * (o ? o->step : 1);
  pace = f.foes * (o ? o->foes : 1) * game.balance.enemySpeed;
  xpMul = f.xp * (o ? o->xp : 1) * game.balance.xp;
  eliteMul = o ? o->elite : 1;
  kindleRate = (o ? o->kindle : 1) * (player->hero ? player->hero->devotion() : 1) *
               (1 + player->bonus.kindling / 100);
  if (streak > 0 && (streakTime -= DT) <= 0)
    streak = 0;
  if (player->dead())
    return;
  player->hp = std::min(player->maxHp, player->hp + f.mend * DT);
  if (f.burn && (burnTime -= DT) <= 0) {
    burnTime = 0.6f;
    quake(player->x, player->y, 110, 4, 10);
  }
}

void Level::hunt() {
  if ((bountyTime -= DT) > 0)
    return;
  bountyTime = 0.5f;
  for (Bounty &b : bounties) {
    if (b.done || b.progress(*this) < b.goal)
      continue;
    b.done = true;
    double dir = std::atan2(player->yAim, player->xAim);
    add<Chest>(player->x + std::cos(dir) * 40, player->y + std::sin(dir) * 40);
    game.announce("BOUNTY: " + b.text());
  }
}

void Level::weather() {
  BiomeId here = world::biomeUnder(player->x, player->y);
  BIOMES[here]->weather(*this, player->x, player->y);
  const Air &a = AIRS[here];
  float shelter = 1 - 0.75f * sky.rough();
  for (float n = a.rate * shelter * (night.dawn ? 0.5f : 1); n > 0; n--) {
    if (Drift::alive > Drift::MAX / 2 || !rng.chance(std::min(1.0f, n)))
      break;
    double x = player->x + rng.range(-260, 260),
           y = player->y + rng.range(-170, 170) - a.vy * 0.3;
    Drift *d = add<Drift>(x, y, a.vx * rng.range(0.8f, 1.2f),
                          a.vy * rng.range(0.8f, 1.2f), a.icon, a.tint,
                          a.life * rng.range(0.7f, 1.2f));
    d->drag = 0, d->sway = a.sway, d->scale = a.scale * rng.range(0.85f, 1.1f);
    d->spin = a.spin * rng.range(-1, 1);
    d->xa += sky.windX * 0.5f, d->ya += sky.windY * 0.5f;
    if (a.spin == 0)
      d->rot = 0;
  }
  sky.tick(*this);
}

float Level::strike() {
  Enemy *target = nullptr;
  int seen = 0;
  enemiesNear(player->x, player->y, 200, [&](Enemy &e) {
    if (!e.boss && rng.chance(1.0f / ++seen))
      target = &e;
  });
  double x = target ? target->x : player->x + rng.range(-200, 200),
         y = target ? target->y : player->y + rng.range(-120, 120);
  double xd = x - player->x, yd = y - player->y;
  if (!target && xd * xd + yd * yd < 60 * 60)
    return -1;
  std::vector<ArcNode> bolt;
  for (int i = 0; i <= 6; i++)
    bolt.push_back(
        {x + (i > 0 && i < 6 ? rng.range(-9, 9) : 0), y - 170 + i * 170 / 6.0});
  Color storm{0.75f, 0.85f, 1, 1};
  add<Arc>(std::move(bolt), 0.3f, storm);
  add<Shockwave>(x, y, 4, 34, 0.35f, storm);
  Spark::burst(*this, x, y, storm, 6, 70, true);
  quake(x, y, 34, 28 + time * 0.08f, 60);
  return float(std::sqrt(xd * xd + yd * yd));
}

void Level::quake(double x, double y, double r, float damage, float force) {
  enemiesNear(x, y, float(r), [&](Enemy &e) {
    double xd = e.x - x, yd = e.y - y;
    if (xd * xd + yd * yd < r * r)
      e.hurt(damage, xd, yd, force);
  });
}

void Level::explore() {
  int found = chart.tick(player->x, player->y);
  if (found >= 0 && chart.discovered > 1)
    game.announce(Chart::title(BiomeId(found)));
  if (found >= 0)
    game.codex.biomes |= 1u << found;
  Structure s;
  int tx = int(std::floor(player->x / 16)),
      ty = int(std::floor(player->y / 16));
  if (!world::structureAt(tx, ty, s) || visited(s))
    return;
  visits.insert(structureKey(&s));
  landmarks += s.kind >= StructureKind::AncientTree;
  entered |= 1u << int(s.kind);
  game.codex.entered[int(s.kind)]++;
  const EncounterDef *e = &ENCOUNTERS[int(s.kind)];
  if (!expansionEncounter(*this, s)) {
    game.announce(e->banner);
    e->enter(this, &s);
  }
  spdlog::info("encounter: {} at tile ({}, {})", s.name(), s.x, s.y);
}

std::vector<Mob *> Level::nearestFoes(bool hostile, double x, double y,
                                      float range, int n) {
  std::vector<Mob *> out;
  if (hostile) {
    friends(x, y, range, [&](Mob &m) { out.push_back(&m); });
    auto d2 = [&](Mob *m) { return (m->x - x) * (m->x - x) + (m->y - y) * (m->y - y); };
    std::sort(out.begin(), out.end(), [&](Mob *a, Mob *b) { return d2(a) < d2(b); });
    if (int(out.size()) > n)
      out.resize(size_t(std::max(0, n)));
    return out;
  }
  for (Enemy *e : nearest(x, y, range, n))
    out.push_back(e);
  return out;
}

void Level::foesOf(const Mob &self, double x, double y, double r,
                   const std::function<void(Mob &)> &fn) {
  if (!self.hostile()) {
    enemiesNear(x, y, float(r), [&](Enemy &e) { fn(e); });
    return;
  }
  friends(x, y, float(r), fn);
}

