#pragma once

#include <SDL3/SDL.h>

#include <vector>

#include "gfx/Screen.hpp"
#include "menu/Menu.hpp"

class Player;
struct Item;

class StashMenu : public Menu {
public:
  static constexpr int COLS = 8, ROWS = 4, CELLS = COLS * ROWS;

  using Menu::Menu;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
  float pauseEffect() const override { return 0.6f; }

  int selected = 0;
  void activate();

private:
  Player &player() const;
  std::vector<Item> &stash() const;
  std::vector<Item> &side(int i) const;
  SDL_FRect cell(int i) const;
  void move(int dx, int dy);
};
