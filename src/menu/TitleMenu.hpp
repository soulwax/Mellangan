#pragma once

#include "menu/Menu.hpp"

class TitleMenu : public Menu {
public:
  using Menu::Menu;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
};
