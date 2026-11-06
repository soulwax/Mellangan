#pragma once

#include <string>
#include <string_view>

struct Settings {
  int master = 100, music = 100, effects = 100, ambience = 100, shake = 100, flash = 100,
      reduced = 0, fullscreen = 0;

  void parse(std::string_view text);
  std::string save() const;
};
