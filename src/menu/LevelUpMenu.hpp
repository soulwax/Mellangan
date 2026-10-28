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
  float pauseEffect() const override { return 1.0f; }

private:
  std::vector<Upgrade *>
      choices; // nullptr: every upgrade is maxed, offer a feast
  int selected = 0;

  bool treasure = false;
  int evolveSlot = -1;

  std::array<SDL_FRect, 4> cards() const;
  void choose(int i);
};
