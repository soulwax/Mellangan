#include "meta/Codex.hpp"

#include "meta/Saves.hpp"

#include <spdlog/fmt/fmt.h>

#include <bit>
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
  if (const char *h = std::strstr(at, "heat "))
    std::sscanf(h + 5, "%d", &heat);
  else
    heat = dawns > 0;
  if (const char *f = std::strstr(at, "feats ")) {
    unsigned long long bits = 0;
    if (std::sscanf(f + 6, "%llu", &bits) == 1)
      feats = bits;
  }
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
  out += fmt::format("\n{}\nhints {}\nfeats {}\nheat {}\nversion {}\n", shrines, hints, feats,
                     heat, VERSION);
  Saves::store(path, out);
}
