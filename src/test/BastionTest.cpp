#undef NDEBUG
#include <cassert>

#include <chrono>
#include <cmath>
#include <iostream>
#include <unordered_map>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "campaign/Bastion.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Sentry.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/pickup/Gem.hpp"
#include "level/Level.hpp"
#include "level/Prop.hpp"
#include "level/World.hpp"

namespace {

using namespace bastion;
constexpr int SDX[4] = {0, 1, 0, -1}, SDY[4] = {-1, 0, 1, 0};

bool solid(int tx, int ty) {
  Prop p;
  return world::propAt(tx, ty, p) && p.solid > 0;
}

} // namespace

void testBastion() {
  Game game;
  Campaign c;
  c.world = "BASTION1";
  game.level = std::make_unique<Level>(game, c.rules());
  Level &level = *game.level;
  c.enter(level);
  level.tick();
  Player &p = *level.player;
  p.god = true;
  level.each<Enemy>([](Enemy &e) { e.remove(); });
  level.tick();

  assert(count() == 1);
  const Field &f = field(0);
  int cx = f.cx, cy = f.cy, open = gateOf(cx, cy);
  for (int s = 0; s < 4; s++)
    for (int k = -GAP; k <= GAP; k++) {
      int tx = cx + SDX[s] * W + (SDX[s] ? 0 : k), ty = cy + SDY[s] * H + (SDY[s] ? 0 : k);
      assert(solid(tx, ty) == (s != open));
    }

  int reachable = 0, open_cells = 0;
  for (int i = 0; i < N * N; i++) {
    open_cells += f.dir[size_t(i)] != WALL;
    reachable += f.dir[size_t(i)] < 8 || f.dir[size_t(i)] == GOAL;
  }
  assert(reachable == open_cells);

  double sx = cx * 16 + 8.0 - SDX[open] * 250, sy = cy * 16 + 13.0 - SDY[open] * 250;
  int steps = 0;
  for (uint8_t d = f.at(sx, sy); d != GOAL && steps < 4000; d = f.at(sx, sy), steps++) {
    assert(d < 8 && !f.inside(sx, sy));
    constexpr float UX[8] = {1, 0.7071f, 0, -0.7071f, -1, -0.7071f, 0, 0.7071f};
    constexpr float UY[8] = {0, 0.7071f, 1, 0.7071f, 0, -0.7071f, -1, -0.7071f};
    sx += UX[d] * 2, sy += UY[d] * 2;
  }
  assert(steps < 4000 && std::hypot(sx - f.gate.x, sy - f.gate.y) < 40);

  double hx = cx * 16 + 8.0, hy = cy * 16 + 13.0;
  double ex = hx - SDX[open] * 200 + SDY[open] * 30, ey = hy - SDY[open] * 200 + SDX[open] * 30;
  double xw = (hx - ex) * 0.1, yw = (hy - ey) * 0.1;
  route(ex, ey, hx, hy, 40, xw, yw);
  assert(std::abs(std::hypot(xw, yw) - 40) < 0.5 && (xw * (hx - ex) + yw * (hy - ey)) < 0.99 * 40 * std::hypot(hx - ex, hy - ey));
  double aw = -(hx - ex), bw = -(hy - ey), a0 = aw, b0 = bw;
  route(ex, ey, hx, hy, 40, aw, bw);
  assert(aw == a0 && bw == b0);
  double ow = 3, pw = 4;
  route(ex, ey, ex + 50, ey, 40, ow, pw);
  assert(ow == 3 && pw == 4);

  int sealed = (open + 2) % 4;
  double wx = cx * 16 + 8.0 + SDX[sealed] * W * 16, wy = cy * 16 + 13.0 + SDY[sealed] * H * 16;
  double ox = wx + SDX[sealed] * 6, oy = wy + SDY[sealed] * 6, nx = wx - SDX[sealed] * 6,
         ny = wy - SDY[sealed] * 6;
  wall(ox, oy, nx, ny);
  assert(!f.inside(nx, ny));
  double gx = f.gate.x + f.gate.nx * 6, gy = f.gate.y + f.gate.ny * 6, ix = f.gate.x - f.gate.nx * 6,
         iy = f.gate.y - f.gate.ny * 6;
  wall(gx, gy, ix, iy);
  assert(f.inside(ix, iy));
  double tx = hx + 10, ty = hy;
  wall(hx + 900, hy, tx, ty);
  assert(!f.inside(tx, ty));

  p.x = hx, p.y = hy;
  std::unordered_map<entt::entity, std::pair<double, double>> last;
  for (int i = 0; i < 16; i++) {
    auto made = Enemy::create(i % 4 == 0 ? EnemyKind::Bat : EnemyKind::Rat, 1, 1);
    made->x = hx - SDX[open] * 220 + (i - 8) * 14 * (SDX[open] ? 0 : 1),
    made->y = hy - SDY[open] * 220 + (i - 8) * 14 * (SDY[open] ? 0 : 1);
    made->aware = true;
    level.add(std::move(made));
  }
  level.tick();
  int crossed = 0, illegal = 0;
  for (int t = 0; t < 60 * 40; t++) {
    p.x = hx, p.y = hy, p.hp = p.maxHp;
    level.tick();
    level.each<Enemy>([&](Enemy &e) {
      auto it = last.find(e.id);
      if (it != last.end()) {
        auto [px, py] = it->second;
        if (!f.inside(px, py) && f.inside(e.x, e.y)) {
          crossed++;
          if (!f.corridor(px, py) && !f.corridor(e.x, e.y))
            illegal++;
        }
      }
      last[e.id] = {e.x, e.y};
    });
  }
  int kills = 0;
  level.each<Sentry>([&](Sentry &s) {
    kills += s.kills;
    assert(std::hypot(s.x - s.post.x, s.y - s.post.y) <= Sentry::GUARD);
  });
  assert(illegal == 0 && int(level.count<Sentry>()) == Sentry::PER_TOWN && kills > 0);

  {
    auto made = Enemy::create(EnemyKind::Rat, 1, 1);
    Enemy *rat = made.get();
    rat->x = hx - SDX[open] * 200, rat->y = hy - SDY[open] * 200;
    level.add(std::move(made));
    level.tick();
    rat->x = hx + 3, rat->y = hy + 3;
    level.tick();
    assert(!f.inside(rat->x, rat->y));
    rat->remove();
  }
  for (auto &up : p.upgrades)
    up->level = 0;
  level.rules.spawnRate = 0, level.rules.dayRate = 0;
  level.each<Enemy>([](Enemy &e) { e.remove(); });
  level.tick();
  int before = kills, breached = 0, peak = 0, levelKills = level.kills;
  int gemsBefore = 0;
  for (auto [id, o] : level.reg.view<Owned>().each())
    gemsBefore += dynamic_cast<Gem *>(o.self.get()) && !o.self->removed;
  level.each<Sentry>([](Sentry &k) { k.kills = 0; });
  before = 0;
  for (int w = 0; w < 3; w++) {
    level.spawner.wave(w, 40);
    for (int t = 0; t < 60 * 30; t++) {
      p.x = hx, p.y = hy, p.hp = p.maxHp;
      level.tick();
      int inside = 0;
      level.each<Enemy>([&](Enemy &e) { inside += f.inside(e.x, e.y) && !f.corridor(e.x, e.y); });
      breached = std::max(breached, inside);
      peak = std::max(peak, int(level.count<Enemy>()));
    }
  }
  for (int t = 0; t < 60 * 20; t++) {
    p.x = hx, p.y = hy, p.hp = p.maxHp;
    level.tick();
  }
  int held = 0;
  level.each<Sentry>([&](Sentry &s) { held += s.kills; });
  int left = 0;
  level.each<Enemy>([&](Enemy &e) { left += std::hypot(e.x - hx, e.y - hy) < 260; });
  if (breached || held - before < 90 || left > 3)
    spdlog::error("bastion waves: {} breached, {} held, {} left near town", breached,
                  held - before, left);
  assert(breached == 0 && held - before >= 90 && left <= 3);
  int gems = 0;
  for (auto [id, o] : level.reg.view<Owned>().each())
    gems += dynamic_cast<Gem *>(o.self.get()) && !o.self->removed;
  int credited = level.kills - levelKills;
  assert(gems <= gemsBefore + 2 * credited && credited * 3 < held);
  int gold = p.gold;
  level.night.dawn = false;
  c.tick(level);
  level.night.dawn = true;
  c.tick(level);
  int tally = 0;
  level.each<Sentry>([&](Sentry &k) { tally += k.kills; });
  assert(tally == 0 && p.gold == gold + held * Campaign::HONOUR_GOLD);
  kills = held;

  {
    Sentry *k = nullptr;
    level.each<Sentry>([&](Sentry &s) { k = k ? k : &s; });
    double hug = f.north + 20;
    auto made = Enemy::create(EnemyKind::Wolf, 1, 1);
    Enemy *wolf = made.get();
    wolf->x = f.west - 1, wolf->y = hug;
    level.add(std::move(made));
    level.tick();
    level.tick();
    wolf->x = f.west - 1, wolf->y = hug;
    k->x = f.west - 4, k->y = hug, k->swing = 1;
    k->tick();
    assert(!f.inside(wolf->x, wolf->y));
    wolf->remove();
    k->x = f.west + 4, k->y = hug, k->swing = 1;
    for (int i = 0; i < 120; i++) {
      double was = k->x, wasY = k->y;
      k->tick();
      assert(f.inside(was, wasY) == f.inside(k->x, k->y) || f.corridor(was, wasY) || f.corridor(k->x, k->y));
    }
  }

  double xr = ex, yr = ey;
  auto start = std::chrono::steady_clock::now();
  double sink = 0;
  for (int i = 0; i < 1000000; i++) {
    double a = (hx - xr) * 0.1, b = (hy - yr) * 0.1;
    route(xr + (i & 63), yr + ((i >> 6) & 63), hx, hy, 40, a, b);
    sink += a + b;
  }
  double ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start).count() / 1e6;
  start = std::chrono::steady_clock::now();
  for (int i = 0; i < 100; i++)
    clear(), raise(cx, cy);
  double us = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - start).count() / 100;
  c.leave();
  std::cout << "[TEST] Bastion: one random bridge, three sealed gates, a flow field that reaches "
               "every open cell and leads the far side to the bridge, steering only toward a "
               "hero across the wall, no foe crossing the fence in a 40 s siege (" << crossed
            << " crossed at the bridge), knights holding three waves alone and killing "
            << kills << "; route " << ns
            << " ns/call, field build " << us << " us" << (sink > 1e300 ? "!" : "") << ": PASS"
            << std::endl;
}
