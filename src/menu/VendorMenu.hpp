#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "gfx/Screen.hpp"
#include "menu/Menu.hpp"

class Player;
struct Vendor;

class VendorMenu : public Menu {
public:
  static constexpr int COLS = 8, ROWS = 4;

  using Menu::Menu;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
  float pauseEffect() const override { return 0.6f; }

  int selected = 0;
  void activate();

private:
  std::string told;
  Color toldColor{};
  float toldTime = 0;

  Player &player() const;
  Vendor &vendor() const;
  int wares() const;
  int slots() const { return wares() + COLS * ROWS; }
  SDL_FRect cell(int i) const;
  void move(int dx, int dy);
  void tell(std::string text, Color c);
};
