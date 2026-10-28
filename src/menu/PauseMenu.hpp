#pragma once

#include "menu/Menu.hpp"

class PauseMenu : public Menu {
public:
  using Menu::Menu;
  void tick() override;
  void render(Screen &s) override;
  float pauseEffect() const override { return 1.0f; }
};
