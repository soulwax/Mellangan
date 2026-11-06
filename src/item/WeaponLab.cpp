#include "item/WeaponLab.hpp"

#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"

namespace {
constexpr int CASES = Upgrade::MAX_LEVEL + 1, TICKS = 1800;
constexpr float SECONDS = TICKS * DT;

enum class Scene { Standing, Swarm };

constexpr const char *SCENES[] = {"42 standing dummies", "a crowd of 40 knights walking in"};

void standing(Level &l, Player &p) {
  constexpr double RADII[] = {24, 40, 95, 150};
  constexpr int COUNTS[] = {6, 8, 12, 16};
  for (int ring = 0; ring < 4; ring++)
    for (int i = 0, n = COUNTS[ring]; i < n; i++) {
      double a = TAU * (i + 0.5 * ring) / n, r = RADII[ring];
      auto made = Enemy::create(EnemyKind::Knight, 1e9f, 1);
      made->x = p.x + std::cos(a) * r, made->y = p.y + std::sin(a) * r;
      made->aware = false, made->plan = nullptr;
      l.add(std::move(made));
    }
}

void swarm(Level &l, Player &p) {
  constexpr int CROWD = 40;
  constexpr double EDGE = 210;
  int alive = 0;
  l.enemiesNear(p.x, p.y, 2 * EDGE, [&](Enemy &e) { alive += !e.dead() && !e.removed; });
  for (; alive < CROWD; alive++) {
    double a = rng.range(0.0f, float(TAU));
    auto made = Enemy::create(EnemyKind::Knight, 3, 1);
    made->x = p.x + std::cos(a) * EDGE, made->y = p.y + std::sin(a) * EDGE;
    l.add(std::move(made));
  }
}

float measure(const char *name, int level, bool evolve, Scene scene) {
  rng.state = 0x9E3779B97F4A7C15ull ^ std::hash<std::string>{}(name);
  Game game;
  game.level = std::make_unique<Level>(game);
  Level &l = *game.level;
  Player &p = *l.player;
  l.rules.spawnRate = 0, l.rules.dayRate = 0, l.xpMul = 0;
  p.god = true;
  for (auto &up : p.upgrades)
    up->level = 0;
  Upgrade *w = p.grant(name, level);
  if (!w)
    return -1;
  if (evolve)
    w->evolve();
  if (scene == Scene::Standing)
    standing(l, p);
  else
    swarm(l, p);
  l.tick(), l.tick();
  w->dealt = 0;
  for (int t = 0; t < TICKS; t++) {
    if (scene == Scene::Swarm && t % 15 == 0)
      swarm(l, p);
    l.tick();
  }
  float dps = w->dealt / SECONDS;
  game.level.reset();
  return dps;
}
} // namespace

int weaponLab(const char *csv) {
  struct Row {
    std::string name;
    float dps[CASES];
  };
  std::string out = "scene,weapon,l1,l2,l3,l4,l5,evolved\n";
  float medians[2] = {};
  std::vector<Row> tables[2];
  for (Scene scene : {Scene::Standing, Scene::Swarm}) {
    std::vector<Row> &rows = tables[int(scene)];
    for (auto &up : Upgrade::all()) {
      if (!up->weapon || up->rare || up->skill)
        continue;
      Row r{up->name, {}};
      for (int lvl = 1; lvl <= Upgrade::MAX_LEVEL; lvl++)
        r.dps[lvl - 1] = measure(up->name, lvl, false, scene);
      r.dps[CASES - 1] = up->evolvedName ? measure(up->name, Upgrade::MAX_LEVEL, true, scene) : 0;
      rows.push_back(r);
    }
    std::vector<float> top;
    for (const Row &r : rows)
      if (r.dps[Upgrade::MAX_LEVEL - 1] > 0)
        top.push_back(r.dps[Upgrade::MAX_LEVEL - 1]);
    std::sort(top.begin(), top.end());
    float median = medians[int(scene)] = top.empty() ? 0 : top[top.size() / 2];

    std::printf("\n%-24s %7s %7s %7s %7s %7s %8s  notes (dps against %s, %.0f s)\n", "WEAPON",
                "L1", "L2", "L3", "L4", "L5", "EVOLVED", SCENES[int(scene)], SECONDS);
    int flagged = 0;
    for (const Row &r : rows) {
      std::string notes;
      for (int i = 1; i < Upgrade::MAX_LEVEL; i++)
        if (r.dps[i - 1] > 0 && r.dps[i] < r.dps[i - 1] * 1.05f)
          notes += fmt::format(" L{} {:+.0f}%", i + 1, 100 * (r.dps[i] / r.dps[i - 1] - 1));
      float l5 = r.dps[Upgrade::MAX_LEVEL - 1];
      if (l5 <= 0)
        notes += " no damage (utility)";
      else if (l5 < median * 0.4f)
        notes += " weak";
      else if (l5 > median * 2.5f)
        notes += " strong";
      if (r.dps[CASES - 1] > 0 && r.dps[CASES - 1] < l5 * 1.3f)
        notes += fmt::format(" evolution {:+.0f}%", 100 * (r.dps[CASES - 1] / l5 - 1));
      flagged += !notes.empty();
      std::printf("%-24s %7.0f %7.0f %7.0f %7.0f %7.0f %8.0f %s\n", r.name.c_str(), r.dps[0],
                  r.dps[1], r.dps[2], r.dps[3], r.dps[4], r.dps[5], notes.c_str());
      out += fmt::format("{},{},{:.1f},{:.1f},{:.1f},{:.1f},{:.1f},{:.1f}\n",
                         scene == Scene::Swarm ? "swarm" : "standing", r.name, r.dps[0], r.dps[1],
                         r.dps[2], r.dps[3], r.dps[4], r.dps[5]);
    }
    std::printf("%zu weapons, median %.0f dps at level 5, %d with notes\n", rows.size(), median,
                flagged);
  }

  std::printf("\n%-24s %9s %9s  (level 5, each as a multiple of its scene's median)\n", "WEAPON",
              "STANDING", "SWARM");
  for (size_t i = 0; i < tables[0].size(); i++) {
    float a = tables[0][i].dps[Upgrade::MAX_LEVEL - 1] / std::max(medians[0], 1.0f);
    float b = tables[1][i].dps[Upgrade::MAX_LEVEL - 1] / std::max(medians[1], 1.0f);
    const char *verdict = a < 0.4f && b < 0.4f   ? "weak in both"
                          : a > 2.5f && b > 2.5f ? "strong in both"
                          : b > a * 2           ? "standing undervalued it"
                          : a > b * 2           ? "standing overvalued it"
                                                : "";
    std::printf("%-24s %8.2fx %8.2fx  %s\n", tables[0][i].name.c_str(), a, b, verdict);
  }
  if (csv) {
    if (FILE *f = std::fopen(csv, "wb")) {
      std::fwrite(out.data(), 1, out.size(), f);
      std::fclose(f);
    }
  }
  return 0;
}
