#include "level/Level.hpp"

#include <spdlog/spdlog.h>

#include <cstring>

#include "Game.hpp"
#include "Prof.h"
#include "Workers.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Encounter.hpp"
#include "level/ExpansionEncounter.hpp"
#include "level/Flavour.hpp"
#include "hero/Hero.hpp"
#include "entity/pickup/Chest.hpp"
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
  for (int y = y0; y <= y0 + before + Screen::H / 16 + 1 + after; y++)
    for (int x = x0; x <= x0 + before + Screen::W / 16 + 1 + after; x++)
      f(x, y);
}

} // namespace

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
  const Flavour &f = FLAVOURS[world::biomeAt(int(std::floor(player->x / 16)),
                                             int(std::floor(player->y / 16)))];
  const Omen *o = omen >= 0 ? &OMENS[omen] : nullptr;
  step = f.step * (o ? o->step : 1);
  pace = f.foes * (o ? o->foes : 1);
  xpMul = f.xp * (o ? o->xp : 1);
  eliteMul = o ? o->elite : 1;
  kindleRate = (o ? o->kindle : 1) * (player->hero ? player->hero->devotion() : 1);
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
  BIOMES[world::biomeAt(int(std::floor(player->x / 16)),
                        int(std::floor(player->y / 16)))]
      ->weather(*this, player->x, player->y);
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
  if (!expansionEncounter(*this,s)) {
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
  eachVisibleTile(s, [&](int x, int y) {
    float n = world::groundShade(x, y), shade = 0.8f + 0.2f * n;
    int own = int(world::biomeAt(x, y));
    Structure st;
    bool protectedGround = world::structureAt(x, y, st) || world::isDirt(x, y) ||
                           BIOMES[own]->fixedGround(x, y);
    if (protectedGround) {
      Color t = world::groundTint(x, y);
      s.color({shade * t.r, shade * 0.97f * t.g,
               (shade * 0.93f + 0.04f * (1 - n)) * t.b, 1});
      s.sprite(world::groundTile(x, y), x * 16, y * 16, 0, 0);
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
      int kind = variant < BIOMES[id]->flowers
                     ? 2
                     : variant < BIOMES[id]->flowers + BIOMES[id]->tufts ? 1 : 0;
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
  });
  s.color(col::white);
}

void Level::scatter(Screen &s) {
  props.clear();
  std::fill(std::begin(solids), std::end(solids), 0.0f);
  solidX = int(std::floor(s.xOffset / 16.0)) - 3,
  solidY = int(std::floor(s.yOffset / 16.0)) - 3;
  Prop p;
  eachVisibleTile(
      s,
      [&](int x, int y) {
        if (!world::propAt(x, y, p))
          return;
        props.push_back(p);
        if (p.solid > 0)
          solids[(y - solidY) * SOLID_W + x - solidX] = p.solid;
      },
      3, 2);
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
  events.render(s,*this);
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
        e->drawShadow(s);
      }
    }
    drawn[k & 0xFFFFFF]->draw(s);
  }
  prof_end(PROF_DRAW);
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
