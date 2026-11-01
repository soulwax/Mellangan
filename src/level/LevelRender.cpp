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
      s.rect(x * 16, y * 16, 16, 4, {0.02f, 0.02f, 0.06f, 0.3f}),
          s.rect(x * 16, y * 16 + 4, 16, 6, {0.02f, 0.02f, 0.06f, 0.12f});
    if (world::heightAt(x - 1, y) > level)
      s.rect(x * 16, y * 16, 3, 16, {0.02f, 0.02f, 0.06f, 0.16f});
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
          shade = (0.8f + 0.2f * n) * (0.94f + 0.08f * world::heightAt(x, y));
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
