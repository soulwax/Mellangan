#include "Bench.hpp"

#include <spdlog/spdlog.h>

#include "Game.hpp"
#include "entity/Player.hpp"

namespace {
const int COUNTS[] = {100, 1000, 10000, 50000, 100000, 200000, 300000, 500000, 1000000};
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

void Bench::frame(Game &g, double dt, int t, double tk, double rd) {
    if (step < 0 || (clock += dt) < WARMUP) return;
    frames++, ticks += t, tickSec += tk, renderSec += rd;
    if (clock < WARMUP + MEASURE || frames < 5) return;
    spdlog::info("bench {:>7} enemies: {:6.1f} fps, tick {:7.2f} ms, render {:7.2f} ms, {} entities", COUNTS[step],
                 frames / (clock - WARMUP), tickSec * 1000 / std::max(ticks, 1), renderSec * 1000 / frames,
                 g.level->reg.view<Owned>().size());
    if (++step < STEPS) return enter(g);
    step = -1;
    g.running = false;
}
