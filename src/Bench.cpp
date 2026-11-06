#include "Bench.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#else
#include <unistd.h>
#include <cstdio>
#endif

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
constexpr int GATE_COUNT = 10000;
constexpr double GATE_FPS = 55, GATE_P99 = 16.7;
constexpr double SOAK_GROWTH = 32;

double memoryMb() {
#ifdef _WIN32
  PROCESS_MEMORY_COUNTERS_EX m{};
  if (!GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&m),
                            sizeof m))
    return 0;
  return double(m.PrivateUsage) / (1 << 20);
#elif defined(__APPLE__)
  task_vm_info_data_t info{};
  mach_msg_type_number_t n = TASK_VM_INFO_COUNT;
  if (task_info(mach_task_self(), TASK_VM_INFO, task_info_t(&info), &n) != KERN_SUCCESS)
    return 0;
  return double(info.phys_footprint) / (1 << 20);
#else
  long pages = 0, resident = 0;
  FILE *f = std::fopen("/proc/self/statm", "r");
  if (!f)
    return 0;
  int got = std::fscanf(f, "%ld %ld", &pages, &resident);
  std::fclose(f);
  return got == 2 ? double(resident) * double(sysconf(_SC_PAGESIZE)) / (1 << 20) : 0;
#endif
}

bool unthrottle() {
#ifdef _WIN32
  PROCESS_POWER_THROTTLING_STATE s{PROCESS_POWER_THROTTLING_CURRENT_VERSION,
                                   PROCESS_POWER_THROTTLING_EXECUTION_SPEED |
                                       PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION,
                                   0};
  return SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &s, sizeof s) != 0;
#else
  return false;
#endif
}
} // namespace

void Bench::start(Game &g) {
  if (unthrottle())
    spdlog::info("bench: power throttling off");
  if (soak) {
    g.newGame();
    spdlog::info("soak: {} minutes of god-mode runs on autopilot", soak);
    return;
  }
  step = 0;
  enter(g);
}

void Bench::drive(Game &g, double dt) {
  static constexpr int DX[8] = {1, 1, 0, -1, -1, -1, 0, 1}, DY[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  Input &in = g.input;
  g.level->player->god = true;
  if ((turn -= dt) <= 0)
    turn = rng.range(2, 6), heading = (heading + rng.irange(-2, 2) + 8) % 8;
  bool walk = !g.menu;
  in.right.toggle(walk && DX[heading] > 0), in.left.toggle(walk && DX[heading] < 0);
  in.down.toggle(walk && DY[heading] > 0), in.up.toggle(walk && DY[heading] < 0);
  press += dt;
  in.confirm.toggle(g.menu && press > 0.5 && press < 0.6);
  if (press >= 0.6)
    press = 0;
}

void Bench::endure(Game &g, double dt, int t, double tk, double rd) {
  drive(g, dt);
  double slot[PROF_COUNT], level = 0;
  for (int i = 0; i < PROF_COUNT; i++)
    level += slot[i] = prof_take(i) * 1000;
  if ((tk + rd) * 1000 > GATE_P99) {
    std::string split;
    for (int i = 0; i < PROF_COUNT; i++)
      if (slot[i] >= 0.5)
        split += fmt::format(" {} {:.1f}", prof_name(i), slot[i]);
    spdlog::warn("soak spike: {:.1f} ms, tick {:.1f} over {} ticks, render {:.1f};{} untracked {:.1f}; "
                 "{} enemies, menu {}",
                 (tk + rd) * 1000, tk * 1000, t, rd * 1000, split, (tk + rd) * 1000 - level,
                 g.level->count<Enemy>(), g.menu != nullptr);
  }
  runs += g.level->time < runTime;
  runTime = g.level->time;
  frames++, clock += dt, costs.push_back((tk + rd) * 1000);
  if (clock < 60.0 * (minute + 1))
    return;
  minute++;
  std::sort(costs.begin(), costs.end());
  double rate = frames / (clock - 60.0 * (minute - 1)), p99 = costs[costs.size() * 99 / 100],
         mem = memoryMb();
  double &peak = runs > 1 ? laterPeak : firstPeak;
  peak = std::max(peak, mem);
  bool stall = minute > 1 && rate < 5,
       bad = minute > 1 && !stall && (rate < GATE_FPS || p99 > GATE_P99);
  slow += bad, stalled += stall;
  spdlog::log(bad || stall ? spdlog::level::warn : spdlog::level::info,
              "soak minute {:>3}{}: {:5.1f} fps, frame p99 {:6.2f} ms, worst {:7.2f} ms, {:7.1f} MB, "
              "{} entities, {} enemies, run {} at {:.0f} s",
              minute, stall ? " (stalled)" : "", rate, p99, costs.back(), mem, g.level->reg.view<Owned>().size(),
              g.level->count<Enemy>(), runs, g.level->time);
  frames = 0, costs.clear();
  if (minute < soak)
    return;
  bool leak = runs > 1 && laterPeak > firstPeak + SOAK_GROWTH;
  failed = slow > 0 || stalled > 0 || leak;
  if (stalled)
    spdlog::warn("soak: {} minutes drew under 5 frames a second although each frame was cheap; "
                 "the window was probably hidden or the screen locked, so rerun with it visible",
                 stalled);
  spdlog::log(failed ? spdlog::level::err : spdlog::level::info,
              "soak {}: {} runs, {} slow minutes (under {:.0f} fps or p99 over {:.1f} ms), "
              "peak memory {:.1f} MB in the first run and {} later (limit +{:.0f} MB)",
              failed ? "FAILED" : "passed", runs, slow, GATE_FPS, GATE_P99, firstPeak,
              runs > 1 ? fmt::format("{:.1f} MB", laterPeak) : std::string("none"), SOAK_GROWTH);
  g.running = false;
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
  costs.clear();
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

void Bench::interrupt() {
  if (!soak || minute >= soak)
    return;
  failed = true;
  spdlog::error("soak interrupted: the window was closed after {} of {} minutes, in run {}", minute,
                soak, runs);
}

void Bench::frame(Game &g, double dt, int t, double tk, double rd) {
  if (soak)
    return endure(g, dt, t, tk, rd);
  if (step < 0)
    return;
  load(g);
  if ((clock += dt) < WARMUP) {
    for (int i = 0; i < PROF_COUNT; i++)
      prof_take(i);
    return;
  }
  frames++, ticks += t, tickSec += tk, renderSec += rd;
  costs.push_back((tk + rd) * 1000);
  if (clock < WARMUP + MEASURE || frames < 5)
    return;
  std::sort(costs.begin(), costs.end());
  double rate = frames / (clock - WARMUP), p99 = costs[costs.size() * 99 / 100];
  spdlog::info("bench {:>7} enemies: {:6.1f} fps, tick {:7.2f} ms, render "
               "{:7.2f} ms, frame p99 {:7.2f} ms, worst {:7.2f} ms, {} entities",
               COUNTS[step], rate, tickSec * 1000 / std::max(ticks, 1),
               renderSec * 1000 / frames, p99, costs.back(),
               g.level->reg.view<Owned>().size());
  std::string split;
  for (int i = 0; i < PROF_COUNT; i++)
    split +=
        fmt::format("  {} {:.2f}", prof_name(i), prof_take(i) * 1000 / frames);
  spdlog::info("       ms/frame:{}  ({} particles, {} debris, {} sparks)",
               split, g.level->particles.size(), int(Debris::alive), int(Spark::alive));
  if (gate && COUNTS[step] == GATE_COUNT) {
    failed = rate < GATE_FPS || p99 > GATE_P99;
    spdlog::log(failed ? spdlog::level::err : spdlog::level::info,
                "bench gate {}: {} enemies need {:.0f} fps and frame p99 under "
                "{:.1f} ms",
                failed ? "FAILED" : "passed", GATE_COUNT, GATE_FPS, GATE_P99);
  }
  if (!(gate && COUNTS[step] >= GATE_COUNT) && ++step < STEPS)
    return enter(g);
  step = -1;
  g.running = false;
}
