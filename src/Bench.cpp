#include "Bench.hpp"

#include <spdlog/spdlog.h>

#include "Game.hpp"
#include "Prof.h"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "particle/Explosion.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/Debris.hpp"

namespace {
const int COUNTS[] = {100,    1000,   10000,  50000,  100000,
                      200000, 300000, 500000, 1000000};
constexpr int STEPS = int(sizeof COUNTS / sizeof *COUNTS);
constexpr double WARMUP = 1, MEASURE = 3;
} // namespace

void Bench::start(Game &g) {
  step = 0;
  enter(g);
}

void Bench::enter(Game &g) {
  g.newGame();
  Level &l = *g.level;
  l.player->god = true;
  for (int i = 0; i < COUNTS[step]; i++) {
    auto e = Enemy::create(EnemyKind(i % int(EnemyKind::Cyclops)), 1e6f, 0);
    double dir = rng.range(0, TAU), r = 40 + 500 * std::sqrt(rng.unit());
    e->x = l.player->x + std::cos(dir) * r;
    e->y = l.player->y + std::sin(dir) * r;
    l.add(std::move(e));
  }
  frames = ticks = 0, clock = tickSec = renderSec = 0;
}

void Bench::load(Game &g) {
  Level &l = *g.level;
  Player &p = *l.player;
  for (int i = 0; i < 6; i++) {
    double dir = rng.range(0, TAU), r = rng.range(20, 200),
           x = p.x + std::cos(dir) * r, y = p.y + std::sin(dir) * r;
    Debris::gore(l, x, y, 7, {0.8f, 0.2f, 0.2f, 1}, i % 3 != 0);
    Spark::burst(l, x, y - 7, {1, 0.7f, 0.3f, 1}, 4, 70, i == 0);
  }
  if (++loads % 15 == 0)
    l.add<Explosion>(p.x + rng.range(-120, 120), p.y + rng.range(-80, 80), 0);
}

void Bench::frame(Game &g, double dt, int t, double tk, double rd) {
  if (step < 0)
    return;
  load(g);
  if ((clock += dt) < WARMUP) {
    for (int i = 0; i < PROF_COUNT; i++)
      prof_take(i);
    return;
  }
  frames++, ticks += t, tickSec += tk, renderSec += rd;
  if (clock < WARMUP + MEASURE || frames < 5)
    return;
  spdlog::info("bench {:>7} enemies: {:6.1f} fps, tick {:7.2f} ms, render "
               "{:7.2f} ms, {} entities",
               COUNTS[step], frames / (clock - WARMUP),
               tickSec * 1000 / std::max(ticks, 1), renderSec * 1000 / frames,
               g.level->reg.view<Owned>().size());
  std::string split;
  for (int i = 0; i < PROF_COUNT; i++)
    split +=
        fmt::format("  {} {:.2f}", prof_name(i), prof_take(i) * 1000 / frames);
  spdlog::info("       ms/frame:{}  ({} particles, {} debris, {} sparks)",
               split, g.level->particles.size(), int(Debris::alive), int(Spark::alive));
               if (++step < STEPS)
                 return enter(g);
               step = -1;
               g.running = false;
}
