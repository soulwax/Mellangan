#include "level/Level.hpp"

#include <cmath>
#include <cstring>
#include <initializer_list>

#include "Game.hpp"
#include "gui/Lang.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/pickup/Pickup.hpp"
#include "level/World.hpp"
#include "particle/Spark.hpp"

namespace {

constexpr Color EMBER{1, 0.45f, 0.12f, 1}, BONE{0.82f, 0.86f, 0.74f, 1}, SAND{0.86f, 0.72f, 0.48f, 1},
                DUST{0.5f, 0.46f, 0.42f, 1}, AMBER{0.95f, 0.7f, 0.3f, 1}, MERE{0.4f, 0.6f, 0.75f, 1},
                WARD{1, 0.85f, 0.45f, 1},
                SPORE{0.72f, 0.42f, 0.86f, 1}, GLINT{0.78f, 0.62f, 1, 1};

constexpr const char *Lang::*DREADS[] = {&Lang::dreadStirs, &Lang::dreadNotices, &Lang::dreadHunts};

void erupt(Level &level, int tx, int ty) {
  int hot[21 * 17], n = 0;
  for (int y = -8; y <= 8; y++)
    for (int x = -10; x <= 10; x++)
      if (world::isLava(tx + x, ty + y))
        hot[n++] = (y + 8) * 21 + x + 10;
  if (!n)
    return;
  int pick = hot[rng.irange(0, n - 1)];
  double cx = (tx + pick % 21 - 10) * 16 + 8.0, cy = (ty + pick / 21 - 8) * 16 + 8.0;
  float damage = level.game.balance.enemyDamage * (8 + level.threatAt(cx, cy) / 40);
  Blast *b = level.add<Blast>(cx, cy, damage, true);
  b->tone = EMBER, b->radius = 34, b->by = "an erupting vent";
}

bool is(const Prop &p, std::initializer_list<const char *> kinds) {
  for (const char *k : kinds)
    if (p.kind && std::strcmp(p.kind, k) == 0)
      return true;
  return false;
}

void rise(Level &level, int tx, int ty, std::initializer_list<const char *> kinds, EnemyKind kind, int count,
          Color tone) {
  int spots[15 * 13], n = 0;
  Prop p;
  for (int y = -6; y <= 6; y++)
    for (int x = -7; x <= 7; x++)
      if (std::abs(x) + std::abs(y) >= 4 && world::propAt(tx + x, ty + y, p) && is(p, kinds))
        spots[n++] = (y + 6) * 15 + x + 7;
  for (int raised = 0, tries = 0; raised < count && n > 0 && tries < count * 2; tries++) {
    int k = rng.irange(0, n - 1), g = spots[k];
    spots[k] = spots[--n];
    double sx = (tx + g % 15 - 7) * 16 + 8.0, sy = (ty + g / 15 - 6) * 16 + 28.0;
    if (level.rockAt(sx, sy - 2) || !level.spawner.spawn(kind, sx, sy, true))
      continue;
    Spark::burst(level, sx, sy - 4, tone, 6, 40, false);
    raised++;
  }
}

void wake(Level &level, int tx, int ty) { rise(level, tx, ty, {"grave"}, EnemyKind::Skeleton, 2, BONE); }

void bones(Level &level, int tx, int ty) {
  double dx = level.player->x - level.caveX, dy = level.player->y - level.caveY;
  if (dx * dx + dy * dy < 64 * 64)
    rise(level, tx, ty, {"ribs", "skull"}, EnemyKind::Skeleton, 3, BONE);
}

void brood(Level &level, int tx, int ty) { rise(level, tx, ty, {"beehive"}, EnemyKind::Spider, 3, AMBER); }

void stir(Level &level, int tx, int ty) {
  int wet[17 * 13], n = 0;
  for (int y = -6; y <= 6; y++)
    for (int x = -8; x <= 8; x++)
      if (std::abs(x) + std::abs(y) >= 3 && world::isWater(tx + x, ty + y))
        wet[n++] = (y + 6) * 17 + x + 8;
  if (!n)
    return;
  int w = wet[rng.irange(0, n - 1)];
  double sx = (tx + w % 17 - 8) * 16 + 8.0, sy = (ty + w / 17 - 6) * 16 + 12.0;
  if (level.spawner.spawn(rng.chance(0.5f) ? EnemyKind::Drowned : EnemyKind::Snapjaw, sx, sy, true))
    Spark::burst(level, sx, sy - 4, MERE, 8, 50, false);
}

bool near(int tx, int ty, int rx, int ry, std::initializer_list<const char *> kinds, int &ox, int &oy) {
  int found[15 * 13], n = 0;
  Prop p;
  for (int y = -ry; y <= ry; y++)
    for (int x = -rx; x <= rx; x++)
      if (world::propAt(tx + x, ty + y, p) && is(p, kinds) && n < 15 * 13)
        found[n++] = (y + ry) * (2 * rx + 1) + x + rx;
  if (!n)
    return false;
  int pick = found[rng.irange(0, n - 1)];
  ox = tx + pick % (2 * rx + 1) - rx, oy = ty + pick / (2 * rx + 1) - ry;
  return true;
}

void snag(Level &level, int, int) {
  Player &p = *level.player;
  double dx = p.xa, dy = p.ya, len = std::sqrt(dx * dx + dy * dy);
  if (len < 10)
    dx = p.xAim, dy = p.yAim, len = std::sqrt(dx * dx + dy * dy);
  if (len < 1e-3)
    return;
  dx /= len, dy /= len;
  double cx = p.x + dx * 52, cy = p.y + dy * 52;
  for (int k = -2; k <= 2; k++) {
    double bx = cx - dy * k * 14, by = cy + dx * k * 14;
    if (!level.rockAt(bx, by - 2))
      level.add<Bramble>(bx, by);
  }
}

void spores(Level &level, int tx, int ty) {
  int mx, my;
  if (!near(tx, ty, 7, 6, {"glowshrooms", "toadstools", "mushrooms"}, mx, my))
    return;
  double cx = mx * 16 + 8.0, cy = my * 16 + 14.0;
  for (int k = 0; k < 7; k++) {
    double a = TAU * k / 6, r = k == 6 ? 0 : 16;
    FrostPatch *f = level.add<FrostPatch>(cx + std::cos(a) * r, cy + std::sin(a) * r, true);
    f->kind = StatusKind::Poison, f->tone = SPORE, f->power = 0.5f;
  }
}

void rubble(Level &level, int, int) {
  Player &p = *level.player;
  for (int k = 0; k < 2; k++)
    for (int tries = 0; tries < 8; tries++) {
      double a = rng.range(0, float(TAU)), r = rng.range(20, 70);
      double x = p.x + std::cos(a) * r, y = p.y + std::sin(a) * r;
      if (level.rockAt(x, y - 2))
        continue;
      float damage = level.game.balance.enemyDamage * (6 + level.threatAt(x, y) / 50);
      Blast *b = level.add<Blast>(x, y, damage, true);
      b->tone = SAND, b->radius = 22, b->by = "falling rock";
      break;
    }
}

void resonance(Level &level, int tx, int ty) {
  int cx, cy;
  if (!near(tx, ty, 2, 2, {"crystal", "amethyst", "crystalSpire"}, cx, cy))
    return;
  Player &p = *level.player;
  if (p.hp >= p.maxHp)
    return;
  p.heal(4);
  Spark::burst(level, cx * 16 + 8.0, cy * 16 + 4.0, GLINT, 3, 20, false);
}

void ward(Level &level, int tx, int ty) {
  int sx, sy;
  if (!near(tx, ty, 3, 3, {"rune stone"}, sx, sy))
    return;
  double cx = sx * 16 + 8.0, cy = sy * 16 + 8.0;
  int scared = 0;
  level.enemiesNear(cx, cy, 110, [&](Enemy &e) { scared += !e.boss && e.afflict(StatusKind::Fear, 3, 1); });
  if (scared)
    Spark::burst(level, cx, cy - 8, WARD, 4, 30, false);
}

void gust(Level &level, int, int) {
  Player &p = *level.player;
  float side = rng.chance(0.5f) ? 1.0f : -1.0f;
  p.xa += side * 240;
  for (int k = 0; k < 6; k++)
    Spark::burst(level, p.x - side * 50 + rng.range(-12, 12), p.y - 24 + k * 8, DUST, 1, 90, false);
}

struct Rule {
  void (*act)(Level &level, int tx, int ty);
  float lo, hi;
};

constexpr Rule RULES[int(CaveBiome::Count)] = {
    {snag, 6, 9},     {nullptr, 0, 0},  {nullptr, 0, 0}, {rubble, 7, 11},
    {spores, 5, 8},   {erupt, 2.5f, 5}, {resonance, 2, 2}, {wake, 9, 14},
    {nullptr, 0, 0},  {stir, 6, 10},    {erupt, 2, 4},     {resonance, 3, 3},
    {bones, 4, 6},    {ward, 2, 2},      {gust, 5, 8},       {brood, 7, 10},
};

} // namespace

float Level::threatAt(double x, double y) const {
  return (rules.threat ? rules.threat(x, y) : time) + DEPTH_THREAT[int(world::layer())] + dread * DREAD_THREAT;
}

bool Level::sanctum(double x, double y) const {
  if (world::layer() != world::Layer::Deep)
    return false;
  int tx = world::tileOf(x), ty = world::tileOf(y);
  world::DeepCell d;
  if (!world::deepCell(int(std::floor(double(tx) / world::CAVE_CELL)), int(std::floor(double(ty) / world::CAVE_CELL)), d) ||
      d.biome != CaveBiome::Halls)
    return false;
  double dx = x - (d.hx * 16 + 8.0), dy = y - (d.hy * 16 + 8.0);
  return dx * dx + dy * dy < SANCTUM * SANCTUM;
}

void Level::haunt() {
  if (rules.endsAtDawn || world::layer() == world::Layer::Surface || player->dead())
    return;
  if (sanctum(player->x, player->y)) {
    dread = std::max(0.0f, dread - 60 * DT), dreadSaid = 0;
    player->mend(2 * DT);
    return;
  }
  dread = std::min(DREAD_MAX, dread + DT * (rules.warded && rules.warded(player->x, player->y) ? 0.25f : 1));
  int stage = int(dread / (DREAD_MAX / 3));
  if (stage > dreadSaid && stage <= 3)
    game.announce(lang.*DREADS[stage - 1]), dreadSaid = stage;
}

void Level::caveRules() {
  if (world::layer() == world::Layer::Surface || player->dead())
    return;
  haunt();
  if (chasmy)
    each<Enemy>([&](Enemy &e) {
      if (!e.type.floats && !e.boss && chasmAt(e.x, e.y - 2))
        Spark::burst(*this, e.x, e.y - 4, DUST, 5, 30, false), e.remove();
    });
  int tx = world::tileOf(player->x), ty = world::tileOf(player->y);
  const Rule &r = RULES[int(world::caveBiomeAt(tx, ty))];
  if (!r.act || (caveTime -= DT) > 0)
    return;
  float busy = 1;
  if (r.act == rubble) {
    int crowd = 0;
    enemiesNear(player->x, player->y, 120, [&](Enemy &) { crowd++; });
    busy = crowd > 12 ? 0.5f : 1;
  }
  caveTime = rng.range(r.lo, r.hi) * busy;
  r.act(*this, tx, ty);
  caveX = player->x, caveY = player->y;
}

void Level::shift(world::Layer to, double x, double y) {
  if (to == depth)
    return;
  each<Enemy>([](Enemy &e) { e.remove(); });
  each<Gem>([](Gem &g) { g.magnet = true; });
  if (Owned *o = reg.valid(hoard) ? reg.try_get<Owned>(hoard) : nullptr; o && o->self && !o->self->removed)
    o->self->remove(), looted.erase(hoardKey);
  hoard = entt::null;
  std::vector<Pickup *> carry;
  for (auto [id, o] : reg.view<Owned>().each())
    if (auto *k = dynamic_cast<Pickup *>(o.self.get()); k && !k->removed && !dynamic_cast<Gem *>(k))
      carry.push_back(k);
  charts[int(depth)] = std::move(chart);
  chart = std::move(charts[int(to)]);
  depth = to;
  world::enter(to);
  rules.sky = to == world::Layer::Surface;
  gloom = rules.sky ? 0 : std::max(gloom, 0.45f);
  rock.clear(), flow.clear();
  caveTime = 3;
  player->x = x, player->y = y, player->xa = player->ya = 0;
  each<Ally>([&](Ally &a) { a.x = x + rng.range(-10, 10), a.y = y + rng.range(4, 12); });
  for (Pickup *k : carry)
    k->x = x + rng.range(-12, 12), k->y = y + rng.range(-4, 10);
  quarry();
  int cx = int(std::floor(double(world::tileOf(x)) / world::CAVE_CELL)),
      cy = int(std::floor(double(world::tileOf(y)) / world::CAVE_CELL)), hx, hy;
  if (world::CaveCell c; to == world::Layer::Caves && world::caveCell(cx, cy, c))
    hx = c.hx, hy = c.hy;
  else if (world::DeepCell d; to == world::Layer::Deep && world::deepCell(cx, cy, d))
    hx = d.hx, hy = d.hy;
  else
    return;
  uint64_t key = (uint64_t(uint32_t(cx)) << 32 | uint32_t(cy)) ^ uint64_t(to) << 62;
  if (looted.insert(key).second)
    hoard = add<Chest>(hx * 16 + 8.0, hy * 16 + 12.0)->id, hoardKey = key;
}

