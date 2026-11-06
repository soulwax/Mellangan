#include "meta/Settings.hpp"

#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace {
struct Field {
  const char *key;
  int Settings::*value;
  int hi;
};

const Field FIELDS[] = {
    {"master", &Settings::master, 100},     {"music", &Settings::music, 100},
    {"effects", &Settings::effects, 100},   {"ambience", &Settings::ambience, 100},
    {"shake", &Settings::shake, 100},       {"flash", &Settings::flash, 100},
    {"reduced_particles", &Settings::reduced, 1}, {"fullscreen", &Settings::fullscreen, 1},
    {"damage_numbers", &Settings::numbers, 1},    {"vsync", &Settings::vsync, 1},
    {"colourblind", &Settings::colours, 4},       {"pixel_perfect", &Settings::pixel, 1}};
} // namespace

void Settings::parse(std::string_view text) {
  std::string s(text);
  for (const char *at = s.c_str(); *at;) {
    char key[32] = {};
    int value = 0;
    if (std::sscanf(at, "%31s %d", key, &value) == 2)
      for (const Field &f : FIELDS)
        if (std::strcmp(key, f.key) == 0)
          this->*f.value = std::clamp(value, 0, f.hi);
    const char *end = std::strchr(at, '\n');
    at = end ? end + 1 : at + std::strlen(at);
  }
}

std::string Settings::save() const {
  std::string out;
  for (const Field &f : FIELDS)
    out += fmt::format("{} {}\n", f.key, this->*f.value);
  return out;
}
