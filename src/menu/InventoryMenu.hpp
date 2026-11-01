#pragma once

#include <SDL3/SDL.h>

#include "menu/Menu.hpp"

class Player;
struct Item;

class InventoryMenu : public Menu {
public:
  static constexpr int COLS = 8, ROWS = 4;

  using Menu::Menu;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
  float pauseEffect() const override { return 0.6f; }

  int selected = 0;
  void activate();
  void dropSelected();

private:
  Player &player() const;
  const Item *at(int i) const;
  SDL_FRect cell(int i) const;
  void move(int dx, int dy);
};
