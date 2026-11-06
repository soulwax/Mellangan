#pragma once

#include "menu/Menu.hpp"

class DeadMenu : public Menu {
public:
  using Menu::Menu;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
  Color endGlow() const override { return {0.95f, 0.2f, 0.16f, std::min(1.0f, t * 0.8f)}; }
};
