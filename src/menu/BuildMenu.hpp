#pragma once

#include <string>

#include "gfx/Screen.hpp"
#include "menu/Menu.hpp"

class BuildMenu : public Menu {
public:
  using Menu::Menu;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
  float pauseEffect() const override { return 0.5f; }

  int selected = 0;

private:
  std::string told;
  float toldTime = 0;
};
