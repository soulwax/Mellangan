#pragma once

#include "menu/Menu.hpp"

class DawnMenu : public Menu {
public:
  using Menu::Menu;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
  Color endGlow() const override { return {1, 0.7f, 0.28f, std::min(1.0f, t * 0.5f)}; }
};
