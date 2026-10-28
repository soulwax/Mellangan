#pragma once

#include "menu/Menu.hpp"

class ShrineMenu : public Menu {
public:
  explicit ShrineMenu(Game &game) : Menu(game) {}
  void tick() override;
  void render(Screen &s) override;
  void choose(int choice);
  int selected=3;
};
