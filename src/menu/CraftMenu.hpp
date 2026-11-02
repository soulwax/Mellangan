#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <vector>

#include "gfx/Screen.hpp"
#include "menu/Menu.hpp"

class Player;
struct Formula;

class CraftMenu : public Menu {
public:
  static constexpr int COLS = 8, ROWS = 4;

  using Menu::Menu;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
  float pauseEffect() const override { return 0.6f; }

  int selected = 0, recipe = 0;
  std::vector<size_t> picks;
  void activate();
  void choose(int r);

private:
  std::string told;
  Color toldColor{};
  float toldTime = 0;

  Player &player() const;
  const Formula &chosen() const;
  int recipes() const;
  int slots() const { return recipes() + COLS * ROWS; }
  SDL_FRect cell(int i) const;
  void move(int dx, int dy);
  void pick(size_t k);
  void tell(std::string text, Color c);
};
