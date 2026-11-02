#pragma once

#include "gfx/Screen.hpp"
#include "menu/Menu.hpp"

class QuestMenu : public Menu {
public:
  using Menu::Menu;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
  float pauseEffect() const override { return 0.6f; }
};
