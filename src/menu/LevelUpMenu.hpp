#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <vector>

#include "menu/Menu.hpp"

class Upgrade;

class LevelUpMenu : public Menu {
public:
  explicit LevelUpMenu(Game &game);
  void tick() override;
  void render(Screen &s) override;

private:
  std::vector<Upgrade *>
      choices; // nullptr: every upgrade is maxed, offer a feast
  int selected = 0;

  bool treasure = false;

  std::array<SDL_FRect, 4> cards() const;
  void choose(int i);
};
