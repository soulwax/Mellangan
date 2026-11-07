#include "meta/Codex.hpp"

#include "hero/Hero.hpp"
#include "meta/Heat.hpp"
#include "meta/Saves.hpp"

#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstring>

int Codex::kills() const {
  int n = 0;
  for (int k : slain)
    n += k;
  return n;
}

int Codex::count(uint32_t mask) const { return std::popcount(mask); }

bool Codex::parse(std::string_view text) {
  std::string s(text);
  const char *at = s.c_str();
  int used = 0;
  if (std::sscanf(at, "%u %u %d %d %d %d %f%n", &biomes, &relics, &runs, &dawns,
                  &bestStreak, &start, &bestTime, &used) < 7)
    return false;
  at += used;
  auto row = [&](int *out, int n) {
    if (*at == '\n')
      at++;
    const char *end = std::strchr(at, '\n');
    std::string line(at, end ? end : at + std::strlen(at));
    const char *p = line.c_str();
    for (int i = 0; i < n && std::sscanf(p, "%d%n", &out[i], &used) == 1; i++)
      p += used;
    at = end ? end : at + line.size();
  };
  row(slain, KINDS);
  row(entered, SITES);
  std::sscanf(at, "%d", &shrines);
  if (const char *h = std::strstr(at, "hints "))
    std::sscanf(h + 6, "%u", &hints);
  if (const char *e = std::strstr(at, "endless "))
    std::sscanf(e + 8, "%f", &bestEndless);
  if (const char *h = std::strstr(at, "heat "))
    std::sscanf(h + 5, "%d", &heat);
  else
    heat = dawns > 0;
  if (const char *f = std::strstr(at, "feats ")) {
    unsigned long long bits = 0;
    if (std::sscanf(f + 6, "%llu", &bits) == 1)
      feats = bits;
  }
  start = std::clamp(start, 0, HERO_COUNT - 1), heat = std::clamp(heat, 0, HEAT_MAX);
  runs = std::max(runs, 0), dawns = std::max(dawns, 0), bestStreak = std::max(bestStreak, 0);
  shrines = std::max(shrines, 0);
  for (float *t : {&bestTime, &bestEndless})
    *t = std::isfinite(*t) ? std::max(*t, 0.0f) : 0.0f;
  return true;
}

void Codex::load(const std::string &path) {
  Codex probe;
  std::string s = Saves::recall(path, [](std::string_view t) {
    return !t.empty() && t.back() == '\n' && Codex().parse(t);
  });
  if (probe.parse(s))
    parse(s);
}

void Codex::save(const std::string &path) const {
  std::string out = fmt::format("{} {} {} {} {} {} {:.1f}\n", biomes, relics,
                                runs, dawns, bestStreak, start, bestTime);
  for (int k : slain)
    out += fmt::format("{} ", k);
  out += "\n";
  for (int k : entered)
    out += fmt::format("{} ", k);
  out += fmt::format("\n{}\nhints {}\nfeats {}\nheat {}\nendless {:.1f}\nversion {}\n", shrines,
                     hints, feats, heat, bestEndless, VERSION);
  Saves::store(path, out);
}
