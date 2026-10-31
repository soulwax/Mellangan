#include "level/Level.hpp"
#include "item/Upgrade.hpp"

#include <spdlog/spdlog.h>

#include <cstring>

#include "Game.hpp"
#include "Prof.h"
#include "Workers.hpp"
#include "audio/Sound.hpp"
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
uint32_t orderedBits(float f) {
  uint32_t u;
  std::memcpy(&u, &f, 4);
  return u ^ (uint32_t(int32_t(u) >> 31) | 0x80000000u);
}

void radixSort(std::vector<uint64_t> &a, std::vector<uint64_t> &tmp) {
  static const int SHIFT[3] = {24, 36, 48};
  tmp.resize(a.size());
  for (int shift : SHIFT) {
    uint32_t count[4097] = {};
    for (uint64_t k : a)
      count[(k >> shift & 4095) + 1]++;
    for (int i = 1; i <= 4096; i++)
      count[i] += count[i - 1];
    for (uint64_t k : a)
      tmp[count[k >> shift & 4095]++] = k;
    a.swap(tmp);
  }
}

template <class F>
void eachVisibleTile(const Screen &s, F &&f, int before = 0, int after = 0) {
  int x0 = int(std::floor(s.xOffset / 16.0)) - before,
      y0 = int(std::floor(s.yOffset / 16.0)) - before;
  for (int y = y0; y <= y0 + before + s.vh / 16 + 1 + after; y++)
    for (int x = x0; x <= x0 + before + s.vw / 16 + 1 + after; x++)
      f(x, y);
}

} // namespace

Level::~Level() = default;

Level::Level(Game &game) : game(game) {
  cellStart.assign(GRID * GRID + 1, 0);
  cellFill.assign(GRID * GRID, 0);
  player = add<Player>();
  flush();
  Bounty::roll(bounties, 3);
  if (rng.chance(0.2f)) {
    omen = rng.irange(0, OMEN_COUNT - 1);
    handicaps += (omen == 0 ? 2 : 1);
  }
}

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
}

void Level::tick() {
  time += DT, ticks++;
  prof_begin(PROF_HOUSEKEEP);
  if (!player->dead() && !night.dawn)
    night.tick();
  if (!player->dead() && !night.dawn)
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
  int n = int(thinkers.size());
  workers.run(
      n, 256,
      [](void *ctx, int begin, int end) {
        Enemy **list = static_cast<Enemy **>(ctx);
        for (int i = begin; i < end; i++)
          list[i]->think();
      },
      thinkers.data());
  workers.run(
      n, 256,
      [](void *ctx, int begin, int end) {
        Enemy **list = static_cast<Enemy **>(ctx);
        for (int i = begin; i < end; i++)
          list[i]->move();
      },
      thinkers.data());
  workers.run(
      n, 256,
      [](void *ctx, int begin, int end) {
        Enemy **list = static_cast<Enemy **>(ctx);
        for (int i = begin; i < end; i++)
          list[i]->push();
      },
      thinkers.data());
  workers.run(
      n, 256,
      [](void *ctx, int begin, int end) {
        Enemy **list = static_cast<Enemy **>(ctx);
        for (int i = begin; i < end; i++)
          list[i]->settle();
      },
      thinkers.data());
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
  BiomeId here = world::biomeAt(int(std::floor(player->x / 16)),
                                int(std::floor(player->y / 16)));
  const Flavour &f = FLAVOURS[here];
  const Color &g = AIRS[here].grade;
  float k = 1 - std::exp(-0.9f * DT);
  air = {lerpf(air.r, g.r, k), lerpf(air.g, g.g, k), lerpf(air.b, g.b, k), 1};
  const Omen *o = omen >= 0 ? &OMENS[omen] : nullptr;
  step = f.step * (o ? o->step : 1);
  pace = f.foes * (o ? o->foes : 1) * game.balance.enemySpeed;
  xpMul = f.xp * (o ? o->xp : 1) * game.balance.xp;
  eliteMul = o ? o->elite : 1;
  kindleRate =
      (o ? o->kindle : 1) * (player->hero ? player->hero->devotion() : 1);
  if (streak > 0 && (streakTime -= DT) <= 0)
    streak = 0;
  if (player->dead())
    return;
  player->hp = std::min(player->maxHp, player->hp + f.mend * DT);
  if (f.burn && (burnTime -= DT) <= 0) {
    burnTime = 0.6f;
    enemiesNear(player->x, player->y, 110, [&](Enemy &e) {
      double xd = e.x - player->x, yd = e.y - player->y;
      if (xd * xd + yd * yd < 110 * 110)
        e.hurt(4, xd, yd, 10);
    });
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
  BiomeId here = world::biomeAt(int(std::floor(player->x / 16)),
                                int(std::floor(player->y / 16)));
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
  enemiesNear(x, y, 40, [&](Enemy &e) {
    double ex = e.x - x, ey = e.y - y;
    if (ex * ex + ey * ey < 34 * 34)
      e.hurt(28 + time * 0.08f, ex, ey, 60);
  });
  return float(std::sqrt(xd * xd + yd * yd));
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
  entered |= 1u << int(s.kind);
  game.codex.entered[int(s.kind)]++;
  const EncounterDef *e = &ENCOUNTERS[int(s.kind)];
  if (!expansionEncounter(*this, s)) {
    game.announce(e->banner);
    e->enter(this, &s);
  }
  spdlog::info("encounter: {} at tile ({}, {})", s.name(), s.x, s.y);
}

int Level::gather(double x, double y, float r, Enemy **out, int max) const {
  int own = cell(y, yGrid) * GRID + cell(x, xGrid), n = 0;
  for (int i = cellStart[own]; i < cellStart[own + 1]; i++)
    if (!cells[i]->removed && (out[n++] = cells[i], n == max))
      return n;
  int x0 = cell(x - r, xGrid), x1 = cell(x + r, xGrid);
  int y0 = cell(y - r, yGrid), y1 = cell(y + r, yGrid);
  for (int yc = y0; yc <= y1; yc++)
    for (int xc = x0, c = yc * GRID + xc; xc <= x1; xc++, c++) {
      if (c == own)
        continue;
      for (int i = cellStart[c]; i < cellStart[c + 1]; i++)
        if (!cells[i]->removed && (out[n++] = cells[i], n == max))
          return n;
    }
  return n;
}

void Level::buildGrid() {
  xGrid = player->x - GRID * CELL * 0.5;
  yGrid = player->y - GRID * CELL * 0.5;
  std::fill(cellStart.begin(), cellStart.end(), 0);
  cellOf.clear();
  each<Enemy>([&](Enemy &e) {
    if (e.buried)
      return;
    int c = cell(e.y, yGrid) * GRID + cell(e.x, xGrid);
    cellOf.push_back(c);
    cellStart[c + 1]++;
  });
  for (int i = 1; i <= GRID * GRID; i++)
    cellStart[i] += cellStart[i - 1];
  std::copy(cellStart.begin(), cellStart.end() - 1, cellFill.begin());
  cells.resize(cellStart.back());
  size_t k = 0;
  each<Enemy>([&](Enemy &e) {
    if (!e.buried)
      cells[cellFill[cellOf[k++]]++] = &e;
  });
}

Mob *Level::playerIn(double x, double y, float r) {
  double xd = player->x - x, yd = player->y - y;
  return !player->dead() && xd * xd + yd * yd < double(r) * r ? player : nullptr;
}

std::vector<Mob *> Level::nearestFoes(bool hostile, double x, double y,
                                      float range, int n) {
  std::vector<Mob *> out;
  if (hostile) {
    if (Mob *m = playerIn(x, y, range); m && n > 0)
      out.push_back(m);
    return out;
  }
  for (Enemy *e : nearest(x, y, range, n))
    out.push_back(e);
  return out;
}

std::vector<Enemy *> Level::nearest(double x, double y, float range, int n) {
  std::vector<std::pair<double, Enemy *>> found;
  each<Enemy>([&](Enemy &e) {
    double xd = e.x - x, yd = e.y - y, d = xd * xd + yd * yd;
    if (d < range * range && !e.buried)
      found.emplace_back(d, &e);
  });
  int k = std::min(n, int(found.size()));
  std::partial_sort(found.begin(), found.begin() + k, found.end(),
                    [](auto &a, auto &b) { return a.first < b.first; });
  std::vector<Enemy *> out;
  for (int i = 0; i < k; i++)
    out.push_back(found[i].second);
  return out;
}

void Level::renderFloor(Screen &s) {
  bool snowing = sky.laid == Front::Snow && sky.cover > 0;
  auto covered = [&](int x, int y) { return sky.snowAt(x, y); };
  bool flooded = sky.flood() > 0;
  auto pool = [&](int x, int y, float shade) {
    if (!sky.poolAt(x, y))
      return;
    Sprite ground = world::groundTile(x, y);
    if (BIOMES[BIOME_SNOW]->frost(ground).y == ground.y)
      return;
    int mask = int(sky.poolAt(x, y - 1)) | sky.poolAt(x, y + 1) << 1 |
               sky.poolAt(x - 1, y) << 2 | sky.poolAt(x + 1, y) << 3;
    s.color({0.4f * shade, 0.47f * shade, 0.64f * shade, 0.94f});
    s.sprite(atlas::waterRim(world::rim(mask)), x * 16, y * 16, 0, 0);
  };
  auto settle = [&](int x, int y, float shade) {
    Sprite ground = world::groundTile(x, y),
           white = BIOMES[BIOME_SNOW]->frost(ground);
    if (white.y == ground.y || !covered(x, y))
      return;
    int mask = int(covered(x, y - 1)) | covered(x, y + 1) << 1 |
               covered(x - 1, y) << 2 | covered(x + 1, y) << 3;
    s.color({shade, shade, shade * 1.03f, 1});
    s.sprite(mask == 15 ? white : atlas::snowRim(world::rim(mask)), x * 16,
             y * 16, 0, 0);
  };
  auto wall = [&](int x, int y, float shade) {
    int level = world::heightAt(x, y);
    if (world::heightAt(x, y - 1) > level && !world::isRamp(x, y - 1))
      s.rect(x * 16, y * 16, 16, 4, {0.02f, 0.02f, 0.06f, 0.3f});
    if (level == 0)
      return;
    auto top = [&](int nx, int ny) { return world::heightAt(nx, ny) >= level; };
    int mask = int(top(x, y - 1)) |
               (top(x, y + 1) || world::isRamp(x, y)) << 1 |
               top(x - 1, y) << 2 | top(x + 1, y) << 3;
    static const int MATERIALS[BIOME_COUNT] = {0, 5, 0, 1, 1, 3, 0, 5, 2,
                                               5, 2, 4, 3, 5, 2, 3, 2, 0,
                                               1, 1, 4, 0, 1, 2, 4, 1};
    auto centre = [](int v) { return (v - (v < 0 ? 2 : 0)) / 3 * 3 + 1; };
    int biome = world::biomeAt(centre(x), centre(y)), material = MATERIALS[biome];
    Color t = material ? Color{1, 1, 1, 1} : BIOMES[biome]->tint;
    s.color({shade * t.r * 0.92f, shade * t.g * 0.9f, shade * t.b * 0.9f, 1});
    if (world::isRamp(x, y)) {
      int part = !world::isRamp(x - 1, y)   ? 0
                 : !world::isRamp(x + 1, y) ? 2
                                            : 1;
      s.color({shade, shade, shade, 1});
      s.sprite(atlas::stairs(material, part), x * 16, y * 16, 0, 0);
      return;
    }
    if (mask != 15) {
      s.sprite(atlas::cliff(material, world::rim(mask)), x * 16, y * 16, 0, 0);
      return;
    }
    static const int CORNERS[4][5] = {{-1, 1, 24, 0, 8},
                                      {1, 1, 26, 12, 8},
                                      {-1, -1, 0, 0, 0},
                                      {1, -1, 2, 12, 0}};
    bool ramp = world::isRamp(x, y);
    for (const auto &c : CORNERS) {
      if (top(x + c[0], y + c[1]) || (c[1] > 0 && ramp))
        continue;
      Sprite piece = atlas::cliff(material, c[2]);
      piece.x += float(c[3]), piece.y += float(c[4]),
          piece.w = 4, piece.h = c[1] > 0 ? 8.0f : 4.0f;
      s.sprite(piece, x * 16 + c[3], y * 16 + c[4], 0, 0);
    }
  };
  eachVisibleTile(s, [&](int x, int y) {
    float n = world::groundShade(x, y),
          shade = (0.8f + 0.2f * n) * (0.94f + 0.1f * world::heightAt(x, y));
    int own = int(world::biomeAt(x, y));
    Structure st;
    bool wet = world::isWater(x, y);
    bool protectedGround = wet || world::structureAt(x, y, st) ||
                           world::isDirt(x, y) ||
                           BIOMES[own]->fixedGround(x, y);
    if (protectedGround) {
      Color t = world::groundTint(x, y);
      if (wet)
        shade *= world::isIce(x, y) ? 0.95f : 0.82f;
      s.color({shade * t.r, shade * 0.97f * t.g,
               (shade * 0.93f + 0.04f * (1 - n)) * t.b, 1});
      s.sprite(world::groundTile(x, y), x * 16, y * 16, 0, 0);
      if (snowing)
        settle(x, y, shade);
      wall(x, y, shade);
      return;
    }
    world::GroundBlend blend = world::groundBlend(x, y);
    int order[3], count = 0;
    for (int i = 0; i < blend.count; i++)
      if (int(blend.biome[i]) == own)
        order[count++] = i;
    for (int i = 0; i < blend.count; i++)
      if (int(blend.biome[i]) != own)
        order[count++] = i;
    float cumulative = 0;
    uint32_t roll = hash2(x, y);
    for (int i = 0; i < count; i++) {
      int k = order[i], id = int(blend.biome[k]);
      cumulative += blend.weight[k];
      float alpha = i == 0 ? 1 : blend.weight[k] / cumulative;
      const Color &t = BIOMES[id]->tint;
      s.color({shade * t.r, shade * 0.97f * t.g,
               (shade * 0.93f + 0.04f * (1 - n)) * t.b, alpha});
      int variant = int(roll % 100);
      int kind = variant < BIOMES[id]->flowers                       ? 2
                 : variant < BIOMES[id]->flowers + BIOMES[id]->tufts ? 1
                                                                     : 0;
      if (id == BIOME_DESERT)
        kind = variant < 10 ? 1 : variant < 15 ? 2 : 0;
      else if (id == BIOME_ASHLANDS || id == BIOME_CRYSTAL ||
               id == BIOME_FUNGAL || id == BIOME_BADLANDS ||
               id == BIOME_SAVANNA || id == BIOME_TAIGA || id >= BIOME_SALT)
        kind = variant < 12 ? 1 : variant < 16 ? 2 : 0;
      Sprite ground = blend.count == 1 && id == own
                          ? world::groundTile(x, y)
                          : BIOMES[id]->transitionGround(kind);
      s.sprite(ground, x * 16, y * 16, 0, 0);
    }
    if (flooded)
      pool(x, y, shade);
    if (snowing)
      settle(x, y, shade);
    wall(x, y, shade);
  });
  s.color(col::white);
}

void Level::scatter(Screen &s) {
  props.clear();
  std::fill(std::begin(solids), std::end(solids), 0.0f);
  solidX = int(std::floor(s.xOffset / 16.0)) - 3,
  solidY = int(std::floor(s.yOffset / 16.0)) - 3;
  waterInView = false;
  for (int gy = 0; gy < SOLID_H; gy++)
    for (int gx = 0; gx < SOLID_W; gx++) {
      int x = solidX + gx, y = solidY + gy;
      uint8_t f = world::isWater(x, y) ? world::isIce(x, y) ? 2 : 1 : 0;
      f |= uint8_t(world::heightAt(x, y) << 2 | world::isRamp(x, y) << 4);
      surfaces[gy * SOLID_W + gx] = f;
      waterInView |= (f & 3) != 0;
    }
  Prop p;
  eachVisibleTile(
      s,
      [&](int x, int y) {
        if (!world::propAt(x, y, p))
          return;
        if (sky.snowAt(x, y))
          for (int i = 0; i < p.count; i++)
            p.parts[i].sprite = BIOMES[BIOME_SNOW]->frost(p.parts[i].sprite);
        Structure site;
        if (p.count >= 2 && !world::structureAt(x, y, site))
          p.sway = sky.sway(p.x, p.y, time);
        props.push_back(p);
        if (p.solid > 0)
          solids[(y - solidY) * SOLID_W + x - solidX] = p.solid;
      },
      3, 4);
}

void Level::collide(double &x, double &y, float radius) const {
  int tx = int(std::floor(x / 16)), ty = int(std::floor((y - 3) / 16));
  for (int yt = ty - 1; yt <= ty + 1; yt++)
    for (int xt = tx - 1; xt <= tx + 1; xt++) {
      int gx = xt - solidX, gy = yt - solidY;
      if (gx < 0 || gy < 0 || gx >= SOLID_W || gy >= SOLID_H ||
          solids[gy * SOLID_W + gx] <= 0)
        continue;
      double xc = xt * 16 + 8.0, yc = yt * 16 + 13.0, xd = x - xc, yd = y - yc;
      double min = solids[gy * SOLID_W + gx] + radius, d2 = xd * xd + yd * yd;
      if (d2 >= min * min)
        continue;
      double len = std::sqrt(d2);
      if (len > 1e-4)
        x = xc + xd / len * min, y = yc + yd / len * min;
      else
        x = xc, y = yc + min;
    }
}

int Level::surfaceAt(double x, double y) const {
  int gx = int(std::floor(x / 16)) - solidX,
      gy = int(std::floor((y - 2) / 16)) - solidY;
  if (gx < 0 || gy < 0 || gx >= SOLID_W || gy >= SOLID_H)
    return 0;
  return surfaces[gy * SOLID_W + gx] & 3;
}

Mob *Level::mob(entt::entity id) {
  if (!reg.valid(id))
    return nullptr;
  auto *o = reg.try_get<Owned>(id);
  if (!o || !o->self || o->self->removed)
    return nullptr;
  return dynamic_cast<Mob *>(o->self.get());
}

void Level::foesOf(const Mob &self, double x, double y, double r,
                   const std::function<void(Mob &)> &fn) {
  if (!self.hostile()) {
    enemiesNear(x, y, r, [&](Enemy &e) { fn(e); });
    return;
  }
  double xd = player->x - x, yd = player->y - y;
  if (!player->dead() && xd * xd + yd * yd < r * r)
    fn(*player);
}

int Level::heightAt(double x, double y) const {
  int gx = int(std::floor(x / 16)) - solidX, gy = int(std::floor((y - 2) / 16)) - solidY;
  if (gx < 0 || gy < 0 || gx >= SOLID_W || gy >= SOLID_H)
    return 0;
  return surfaces[gy * SOLID_W + gx] >> 2 & 3;
}

float Level::climb(double x, double y, double dx, double dy) const {
  double d = std::sqrt(dx * dx + dy * dy);
  if (d < 1e-3)
    return 1;
  auto cell = [&](double px, double py) -> int {
    int gx = int(std::floor(px / 16)) - solidX,
        gy = int(std::floor((py - 2) / 16)) - solidY;
    return gx < 0 || gy < 0 || gx >= SOLID_W || gy >= SOLID_H
               ? -1
               : surfaces[gy * SOLID_W + gx];
  };
  int here = cell(x, y), ahead = cell(x + dx / d * 7, y + dy / d * 7);
  if (here < 0 || ahead < 0 || (ahead >> 2 & 3) <= (here >> 2 & 3) ||
      ahead & 16)
    return 1;
  return 0.35f;
}

bool Level::swimming(const SpriteObject *e) const {
  if (!waterInView || surfaceAt(e->x, e->y) != 1)
    return false;
  if (auto *en = dynamic_cast<const Enemy *>(e))
    return !en->type.floats;
  return dynamic_cast<const Player *>(e) != nullptr;
}

bool Level::blocks(double x, double y) const {
  int tx = int(std::floor(x / 16)), ty = int(std::floor(y / 16));
  for (int yt = ty - 1; yt <= ty + 1; yt++)
    for (int xt = tx - 1; xt <= tx + 1; xt++) {
      int gx = xt - solidX, gy = yt - solidY;
      if (gx < 0 || gy < 0 || gx >= SOLID_W || gy >= SOLID_H)
        continue;
      float r = solids[gy * SOLID_W + gx];
      double xd = x - (xt * 16 + 8.0), yd = y - (yt * 16 + 13.0);
      if (r > 0 && xd * xd + yd * yd < r * r)
        return true;
    }
  return false;
}

void Level::render(Screen &s) {
  prof_begin(PROF_FLOOR);
  renderFloor(s);
  sky.renderGround(s);
  events.render(s, *this);
  prof_end(PROF_FLOOR);
  prof_begin(PROF_SORT);
  drawn.clear(), keys.clear();
  for (auto [id, o] : reg.view<Owned>().each())
    if (!o.self->removed && s.visible(o.self->x, o.self->y, 48))
      queue(o.self.get());
  for (auto &p : particles)
    if (!p->removed && showParticle(*p, game.reducedParticles) &&
        s.visible(p->x, p->y, 48))
      queue(p.get());
  for (Prop &p : props)
    queue(&p);
  radixSort(keys, keysTmp);
  prof_end(PROF_SORT);
  if (sky.flood() > 0 || waterInView)
    reflect(s);
  prof_begin(PROF_DRAW);
  bool shadows = false;
  for (uint64_t k : keys) {
    if (!shadows && k >> 56 >= 1) {
      shadows = true;
      for (uint64_t m : keys) {
        if (m >> 56 != 1)
          continue;
        SpriteObject *e = drawn[m & 0xFFFFFF];
        float width, height = e->occlusion(width);
        if (height > 0)
          lights.cast(s, e->x, e->y, width, height);
        if (swimming(e)) {
          int frame = 2 + (int(time * 3 + e->x * 0.1) & 1);
          s.sky(atlas::wx::ripple(frame), e->x, e->y - 2,
                Color{0.75f, 0.85f, 1, 0.55f}, 1.6f, 1.2f);
          continue;
        }
        e->drawShadow(s);
      }
    }
    SpriteObject *e = drawn[k & 0xFFFFFF];
    if (swimming(e)) {
      s.submerge(std::floor(e->y) - 5);
      e->draw(s);
      s.emerge();
    } else
      e->draw(s);
  }
  sky.render(s, air);
  prof_end(PROF_DRAW);
}

void Level::reflect(Screen &s) {
  int shown = 0;
  for (uint64_t k : keys) {
    SpriteObject *e = drawn[k & 0xFFFFFF];
    int tx = int(std::floor(e->x / 16)), ty = int(std::floor(e->y / 16));
    auto wet = [&](int x, int y) {
      return sky.poolAt(x, y) ||
             (surfaces[std::clamp(y - solidY, 0, SOLID_H - 1) * SOLID_W +
                       std::clamp(x - solidX, 0, SOLID_W - 1)] &
              3) == 1;
    };
    if (!wet(tx, ty + 1) || (!wet(tx, ty) && !wet(tx, ty + 2)))
      continue;
    Prop *prop = dynamic_cast<Prop *>(e);
    if (!prop && !dynamic_cast<Entity *>(e))
      continue;
    if (prop && prop->count > 1 && !wet(tx, ty + 2))
      continue;
    bool crowd = dynamic_cast<Enemy *>(e);
    if (crowd && shown >= 48)
      continue;
    s.mirror(std::floor(e->y), {0.55f, 0.66f, 0.9f, crowd ? 0.26f : 0.42f});
    e->draw(s);
    s.unmirror();
    shown += crowd;
  }
  s.color(col::white);
}

void Level::queue(SpriteObject *e) {
  keys.push_back(uint64_t(e->layer()) << 56 |
                 uint64_t(orderedBits(float(e->y))) << 24 | drawn.size());
  drawn.push_back(e);
}

void Level::renderLight(Screen &s) {
  prof_begin(PROF_LIGHT);
  scatter(s);
  lights.clear();
  player->drawLight(s);
  night.renderLight(s);
  sky.renderLight(s);
  for (auto [id, o] : reg.view<Owned>().each())
    if (o.self.get() != player && !o.self->removed &&
        s.visible(o.self->x, o.self->y, 160))
      o.self->drawLight(s);
  for (auto &p : particles)
    if (!p->removed && showParticle(*p, game.reducedParticles) &&
        s.visible(p->x, p->y, 60))
      p->drawLight(s);
  for (Prop &p : props) {
    if (p.light.a <= 0)
      continue;
    if (p.wisp) {
      float f = 0.8f + 0.2f * float(std::sin(time * 2 + p.x * 0.37));
      s.glow(p.x, p.y + p.lightYo + std::sin(time * 1.7 + p.y) * 2, p.lightSize,
             p.light * f);
    } else {
      float f = flicker(time, float(p.x * 0.13 + p.y * 0.07));
      s.glow(p.x, p.y + p.lightYo, p.lightSize * (0.95f + 0.05f * f),
             p.light * f);
      lights.add(p.x, p.y + p.lightYo + 6, p.lightSize * 0.9f, f);
    }
  }
  prof_end(PROF_LIGHT);
}
