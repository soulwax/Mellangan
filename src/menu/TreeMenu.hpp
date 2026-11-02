#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "gfx/Screen.hpp"
#include "item/tree/Tree.hpp"
#include "menu/Menu.hpp"

class Player;

class TreeMenu : public Menu {
public:
  static constexpr int WRAP = 7;

  explicit TreeMenu(Game &game);
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
  float pauseEffect() const override { return 0.6f; }

  Tree tree;
  int selected = 0;
  void activate();

private:
  std::string told;
  float toldTime = 0;

  Player &player() const;
  int &points() const;
  SDL_FRect cell(int i) const;
  float columnX(int tab) const;
  float tierY(int tab, int tier) const;
  void move(int dx, int dy);
};
