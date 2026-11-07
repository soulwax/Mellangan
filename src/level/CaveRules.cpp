#include "level/Level.hpp"

#include <cmath>
#include <cstring>
#include <initializer_list>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/pickup/Pickup.hpp"
#include "level/World.hpp"
#include "particle/Spark.hpp"

namespace {

constexpr Color EMBER{1, 0.45f, 0.12f, 1}, BONE{0.82f, 0.86f, 0.74f, 1}, SAND{0.86f, 0.72f, 0.48f, 1},
                DUST{0.5f, 0.46f, 0.42f, 1},
                SPORE{0.72f, 0.42f, 0.86f, 1}, GLINT{0.78f, 0.62f, 1, 1};

float threatAt(Level &level, double x, double y) {
  return level.rules.threat ? level.rules.threat(x, y) : level.time;
}

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
  float damage = level.game.balance.enemyDamage * (8 + threatAt(level, cx, cy) / 40);
  Blast *b = level.add<Blast>(cx, cy, damage, true);
  b->tone = EMBER, b->radius = 34, b->by = "an erupting vent";
}

void wake(Level &level, int tx, int ty) {
  int graves[15 * 13], n = 0;
  Prop p;
  for (int y = -6; y <= 6; y++)
    for (int x = -7; x <= 7; x++)
      if (std::abs(x) + std::abs(y) >= 4 && world::propAt(tx + x, ty + y, p) && p.kind &&
          std::strcmp(p.kind, "grave") == 0)
        graves[n++] = (y + 6) * 15 + x + 7;
  for (int raised = 0, tries = 0; raised < 2 && n > 0 && tries < 4; tries++) {
    int k = rng.irange(0, n - 1), g = graves[k];
    graves[k] = graves[--n];
    double sx = (tx + g % 15 - 7) * 16 + 8.0, sy = (ty + g / 15 - 6) * 16 + 28.0;
    if (level.rockAt(sx, sy - 2) || !level.spawner.spawn(EnemyKind::Skeleton, sx, sy, true))
      continue;
    Spark::burst(level, sx, sy - 4, BONE, 6, 40, false);
    raised++;
  }
}

bool near(int tx, int ty, int rx, int ry, std::initializer_list<const char *> kinds, int &ox, int &oy) {
  int found[15 * 13], n = 0;
  Prop p;
  for (int y = -ry; y <= ry; y++)
    for (int x = -rx; x <= rx; x++) {
      if (!world::propAt(tx + x, ty + y, p) || !p.kind)
        continue;
      for (const char *k : kinds)
        if (std::strcmp(p.kind, k) == 0 && n < 15 * 13)
          found[n++] = (y + ry) * (2 * rx + 1) + x + rx;
    }
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
      float damage = level.game.balance.enemyDamage * (6 + threatAt(level, x, y) / 50);
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

struct Rule {
  void (*act)(Level &level, int tx, int ty);
  float lo, hi;
};

constexpr Rule RULES[int(CaveBiome::Count)] = {
    {snag, 6, 9},     {nullptr, 0, 0},  {nullptr, 0, 0}, {rubble, 7, 11},
    {spores, 5, 8},   {erupt, 2.5f, 5}, {resonance, 2, 2}, {wake, 9, 14},
    {nullptr, 0, 0},  {nullptr, 0, 0},  {nullptr, 0, 0}, {nullptr, 0, 0},
    {nullptr, 0, 0},  {nullptr, 0, 0},  {nullptr, 0, 0}, {nullptr, 0, 0},
};

} // namespace

void Level::caveRules() {
  if (world::layer() != world::Layer::Caves || player->dead())
    return;
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

