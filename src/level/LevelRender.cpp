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
#include "level/FloorPainter.hpp"
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
  FloorPainter paint(s, sky);
  eachVisibleTile(s, [&](int x, int y) { paint.tile(x, y); });
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
        bool wild = !world::structureAt(x, y, site);
        if (p.count >= 2 && wild)
          p.sway = sky.sway(p.x, p.y, time);
        if (p.kind && wild) {
          uint32_t h = hash2(x * 7 + 3, y * 13 + 5);
          p.flip = h & 1;
          p.tone = 0.86f + float(h >> 8 & 255) / 255 * 0.2f;
          p.warm = (float(h >> 16 & 255) / 255 - 0.5f) * 0.14f;
        }
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
  matter.render(s, time);
  events.render(s, *this);
  prof_end(PROF_FLOOR);
  prof_begin(PROF_SORT);
  drawn.clear(), keys.clear();
  queueVisible(s);
  for (auto &p : particles)
    if (!p->removed && showParticle(*p, game.settings.reduced, game.settings.numbers) &&
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

void Level::queueVisible(const Screen &s) {
  constexpr float MARGIN = 48, SLACK = 64;
  for (auto [id, o] : reg.view<Owned>(entt::exclude<Is<Enemy>>).each())
    if (!o.self->removed && s.visible(o.self->x, o.self->y, MARGIN))
      queue(o.self.get());
  auto look = [&](Enemy &e) {
    if (!e.removed && s.visible(e.x, e.y, MARGIN))
      queue(&e);
  };
  float pad = MARGIN + SLACK;
  enemiesIn(s.xOffset - pad, s.yOffset - pad, s.xOffset + s.vw + pad,
            s.yOffset + s.vh + pad, look);
  for (Enemy *e : loose)
    look(*e);
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
    if (!p->removed && showParticle(*p, game.settings.reduced, game.settings.numbers) &&
        s.visible(p->x, p->y, 60))
      p->drawLight(s);
  for (const Prop &p : props)
    p.shine(s, lights, time);
  prof_end(PROF_LIGHT);
}
