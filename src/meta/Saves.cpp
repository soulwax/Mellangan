#include "meta/Saves.hpp"

#include <SDL3/SDL.h>
#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cctype>
#include <cstdio>

void Saves::open() {
  if (char *pref = SDL_GetPrefPath("soulwax", "Mellangan"))
    path = pref, SDL_free(pref);
}

std::string Saves::slurp(const std::string &file) {
  size_t size = 0;
  char *data = static_cast<char *>(SDL_LoadFile(file.c_str(), &size));
  std::string s = data ? std::string(data, size) : "";
  SDL_free(data);
  return s;
}

std::string Saves::read(const std::string &name) const {
  return slurp(path + name);
}

void Saves::write(const std::string &name, std::string_view data) const {
  SDL_SaveFile((path + name).c_str(), data.data(), data.size());
}

void Saves::loadWorld(WorldMode &mode, std::string &code) {
  std::string s = read("world.txt");
  if (s.rfind("random", 0) == 0)
    mode = WorldMode::Random;
  else if (s.rfind("daily", 0) == 0)
    mode = WorldMode::Daily;
  else if (s.rfind("custom ", 0) == 0)
    mode = WorldMode::Custom, code = s.substr(7, 12);
  while (!code.empty() && !std::isalnum(uint8_t(code.back())))
    code.pop_back();
  std::string all = read("worlds.txt");
  for (size_t at = 0; at < all.size() && int(history.size()) < HISTORY;) {
    size_t end = all.find('\n', at);
    std::string line = all.substr(at, end - at);
    at = end == std::string::npos ? all.size() : end + 1;
    char buf[16] = {};
    PastWorld w{};
    int dawn = 0;
    if (std::sscanf(line.c_str(), "%15s %f %d %d", buf, &w.best, &w.runs,
                    &dawn) == 4)
      w.code = buf, w.dawn = dawn, history.push_back(w);
  }
}

void Saves::saveWorld(WorldMode mode, const std::string &code) const {
  std::string s = mode == WorldMode::Random   ? "random\n"
                  : mode == WorldMode::Daily  ? "daily\n"
                  : mode == WorldMode::Custom ? "custom " + code + "\n"
                                              : "classic\n";
  write("world.txt", s);
}

void Saves::record(const std::string &code, float time, bool dawn) {
  PastWorld w{code, time, 1, dawn};
  for (size_t i = 0; i < history.size(); i++)
    if (history[i].code == code) {
      w.best = std::max(w.best, history[i].best);
      w.runs += history[i].runs, w.dawn |= history[i].dawn;
      history.erase(history.begin() + long(i));
      break;
    }
  history.insert(history.begin(), w);
  while (int(history.size()) > HISTORY) {
    SDL_RemovePath((path + "chart-" + history.back().code + ".bin").c_str());
    history.pop_back();
  }
  std::string out;
  for (const PastWorld &p : history)
    out += fmt::format("{} {:.1f} {} {}\n", p.code, p.best, p.runs, int(p.dawn));
  write("worlds.txt", out);
}
