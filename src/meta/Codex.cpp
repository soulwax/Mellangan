#include "meta/Codex.hpp"

#include <SDL3/SDL.h>
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

void Codex::load(const std::string &path) {
  size_t size = 0;
  char *data = static_cast<char *>(SDL_LoadFile(path.c_str(), &size));
  if (!data)
    return;
  std::string s(data, size);
  SDL_free(data);
  const char *at = s.c_str();
  int used = 0;
  if (std::sscanf(at, "%u %u %d %d %d %d %f%n", &biomes, &relics, &runs,
                  &dawns, &bestStreak, &start, &bestTime, &used) < 7)
    return;
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
}

void Codex::save(const std::string &path) const {
  std::string out = fmt::format("{} {} {} {} {} {} {:.1f}\n", biomes, relics,
                                runs, dawns, bestStreak, start, bestTime);
  for (int k : slain)
    out += fmt::format("{} ", k);
  out += "\n";
  for (int k : entered)
    out += fmt::format("{} ", k);
  out += fmt::format("\n{}\n", shrines);
  SDL_SaveFile(path.c_str(), out.data(), out.size());
}
